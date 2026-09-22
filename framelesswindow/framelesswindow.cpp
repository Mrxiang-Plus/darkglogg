/*
###############################################################################
#                                                                             #
# The MIT License                                                             #
#                                                                             #
# Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)          #
#               >> https://github.com/Jorgen-VikingGod                        #
#                                                                             #
# Sources: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle  #
#                                                                             #
###############################################################################
*/

#include "framelesswindow/framelesswindow.h"
#include <QApplication>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QLineEdit>

#include "poplistview.h"
#include "ui_framelesswindow.h"

FramelessWindow::FramelessWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FramelessWindow),
      m_bMousePressed(false),
      m_bDragTop(false),
      m_bDragLeft(false),
      m_bDragRight(false),
      m_bDragBottom(false) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  // append minimize button flag in case of windows,
  // for correct windows native handling of minimize function
#if defined(Q_OS_WIN)
  setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
#endif
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_TranslucentBackground);

  ui->setupUi(this);
  ui->restoreButton->setVisible(false);

  // shadow under window title text
  QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
  textShadow->setBlurRadius(4.0);
  textShadow->setColor(QColor(0, 0, 0));
  textShadow->setOffset(0.0);
  ui->titleText->setGraphicsEffect(textShadow);

  // window shadow
  QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
  windowShadow->setBlurRadius(9.0);
  windowShadow->setColor(palette().color(QPalette::Highlight));
  windowShadow->setOffset(0.0);
  ui->windowFrame->setGraphicsEffect(windowShadow);

  QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this,
                   &FramelessWindow::on_applicationStateChanged);
  setMouseTracking(true);

  // important to watch mouse move from all child widgets
  QApplication::instance()->installEventFilter(this);

  on_maximizeButton_clicked();
}

FramelessWindow::~FramelessWindow() { delete ui; }

void FramelessWindow::on_restoreButton_clicked() {
  ui->restoreButton->setVisible(false);

  ui->maximizeButton->setVisible(true);
  setWindowState(Qt::WindowNoState);
  // on MacOS this hack makes sure the
  // background window is repaint correctly
  hide();
  show();
}

void FramelessWindow::on_maximizeButton_clicked() {
  ui->restoreButton->setVisible(true);
  ui->maximizeButton->setVisible(false);
  this->setWindowState(Qt::WindowMaximized);
  this->showMaximized();
  styleWindow(true, false);
}

void FramelessWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
    if (windowState().testFlag(Qt::WindowNoState)) {
      ui->restoreButton->setVisible(false);
      ui->maximizeButton->setVisible(true);
      styleWindow(true, true);
      event->ignore();
    } else if (windowState().testFlag(Qt::WindowMaximized)) {
      ui->restoreButton->setVisible(true);
      ui->maximizeButton->setVisible(false);
      styleWindow(true, false);
      event->ignore();
    }
  }
  event->accept();
}
void FramelessWindow::closeEvent(QCloseEvent *event) { mw->close(); }

void FramelessWindow::setContent(MainWindow *w) {
  ui->windowContent->layout()->addWidget(w);
  mw = w;
  connect(this, SIGNAL(addToQuickMark(const QString &)), mw,
          SLOT(appendToQuickMark(const QString &)));
}
WindowDragger *FramelessWindow::getTitleBar() { return ui->windowTitlebar; }
void FramelessWindow::setWindowTitle(const QString &text) {
  ui->titleText->setText(text);
}

void FramelessWindow::setWindowIcon(const QIcon &ico) {
  ui->icon->setPixmap(ico.pixmap(16, 16));
}

void FramelessWindow::styleWindow(bool bActive, bool bNoState) {
  if (bActive) {
    if (bNoState) {
      layout()->setContentsMargins(15, 15, 15, 15);
      ui->windowTitlebar->setStyleSheet(QStringLiteral(
          "#windowTitlebar{border: 0px none palette(shadow); "
          "border-top-left-radius:5px; border-top-right-radius:5px; "
          "background-color:palette(shadow); height:20px;}"));
      ui->windowFrame->setStyleSheet(QStringLiteral(
          "#windowFrame{border:0px none palette(dark); border-radius:5px "
          "5px 5px 5px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->windowFrame->graphicsEffect();
      if (oldShadow) delete oldShadow;
      QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
      windowShadow->setBlurRadius(9.0);
      windowShadow->setOffset(0.0);
      ui->windowFrame->setGraphicsEffect(windowShadow);
    } else {
      layout()->setContentsMargins(0, 0, 0, 0);
      ui->windowTitlebar->setStyleSheet(QStringLiteral(
          "#windowTitlebar{border: 0px none palette(shadow); "
          "border-top-left-radius:0px; border-top-right-radius:0px; "
          "background-color:palette(shadow); height:20px;}"));
      ui->windowFrame->setStyleSheet(QStringLiteral(
          "#windowFrame{border:0px solid palette(dark); border-radius:0px 0px "
          "0px 0px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->windowFrame->graphicsEffect();
      if (oldShadow) delete oldShadow;
      ui->windowFrame->setGraphicsEffect(nullptr);
    }
  } else {
    if (bNoState) {
      layout()->setContentsMargins(15, 15, 15, 15);
      ui->windowTitlebar->setStyleSheet(QStringLiteral(
          "#windowTitlebar{border: 0px none palette(shadow); "
          "border-top-left-radius:5px; border-top-right-radius:5px; "
          "background-color:palette(dark); height:20px;}"));
      ui->windowFrame->setStyleSheet(QStringLiteral(
          "#windowFrame{border:0px solid #000000; border-radius:5px 5px 5px "
          "5px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->windowFrame->graphicsEffect();
      if (oldShadow) delete oldShadow;
      QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
    } else {
      layout()->setContentsMargins(0, 0, 0, 0);
      ui->windowTitlebar->setStyleSheet(QStringLiteral(
          "#titlebarWidget{border: 0px none palette(shadow); "
          "border-top-left-radius:0px; border-top-right-radius:0px; "
          "background-color:palette(dark); height:20px;}"));
      ui->windowFrame->setStyleSheet(QStringLiteral(
          "#windowFrame{border:0px solid palette(shadow); border-radius:0px "
          "0px 0px 0px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->windowFrame->graphicsEffect();
      if (oldShadow) delete oldShadow;
      ui->windowFrame->setGraphicsEffect(nullptr);
    }
  }
}

void FramelessWindow::on_applicationStateChanged(Qt::ApplicationState state) {
  if (windowState().testFlag(Qt::WindowNoState)) {
    if (state == Qt::ApplicationActive) {
      styleWindow(true, true);
    } else {
      styleWindow(false, true);
    }
  } else if (windowState().testFlag(Qt::WindowFullScreen)) {
    if (state == Qt::ApplicationActive) {
      styleWindow(true, false);
    } else {
      styleWindow(false, false);
    }
  }
}

void FramelessWindow::on_minimizeButton_clicked() {
  setWindowState(Qt::WindowMinimized);
}

void FramelessWindow::on_closeButton_clicked() { close(); }

void FramelessWindow::on_windowTitlebar_doubleClicked() {
  if (windowState().testFlag(Qt::WindowNoState)) {
    on_maximizeButton_clicked();
  } else if (windowState().testFlag(Qt::WindowMaximized)) {
    on_restoreButton_clicked();
  }
}

void FramelessWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  Q_UNUSED(event);
}

void FramelessWindow::checkBorderDragging(QMouseEvent *event) {
  if (isMaximized()) {
    return;
  }

  QPoint globalMousePos = event->globalPosition().toPoint();
  if (m_bMousePressed) {
    const int MIN_SIZE = 50;

    // top right corner
    if (m_bDragTop && m_bDragRight) {
      int diff =
          globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
      int neww = m_StartGeometry.width() + diff;
      diff = globalMousePos.y() - m_StartGeometry.y();
      int newy = m_StartGeometry.y() + diff;
      if (neww > MIN_SIZE) {
        QRect newg = m_StartGeometry;
        newg.setWidth(neww);
        newg.setX(m_StartGeometry.x());
        newg.setY(newy);
        setGeometry(newg);
      }
    }
    // top left corner
    else if (m_bDragTop && m_bDragLeft) {
      int diff = globalMousePos.y() - m_StartGeometry.y();
      int newy = m_StartGeometry.y() + diff;
      diff = globalMousePos.x() - m_StartGeometry.x();
      int newx = m_StartGeometry.x() + diff;
      QRect newg = m_StartGeometry;
      newg.setY(newy);
      newg.setX(newx);
      setGeometry(newg);
    }
    // bottom left corner
    else if (m_bDragBottom && m_bDragLeft) {
      int diff =
          globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
      int newh = m_StartGeometry.height() + diff;
      diff = globalMousePos.x() - m_StartGeometry.x();
      int newx = m_StartGeometry.x() + diff;
      if (newh > MIN_SIZE) {
        QRect newg = m_StartGeometry;
        newg.setX(newx);
        newg.setHeight(newh);
        setGeometry(newg);
      }
    } else if (m_bDragTop) {
      int diff = globalMousePos.y() - m_StartGeometry.y();
      int newy = m_StartGeometry.y() + diff;
      QRect newg = m_StartGeometry;
      newg.setY(newy);
      setGeometry(newg);
    } else if (m_bDragLeft) {
      int diff = globalMousePos.x() - m_StartGeometry.x();
      int newx = m_StartGeometry.x() + diff;
      QRect newg = m_StartGeometry;
      newg.setX(newx);
      setGeometry(newg);
    } else if (m_bDragRight) {
      int diff =
          globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
      int neww = m_StartGeometry.width() + diff;
      if (neww > MIN_SIZE) {
        QRect newg = m_StartGeometry;
        newg.setWidth(neww);
        newg.setX(m_StartGeometry.x());
        setGeometry(newg);
      }
    } else if (m_bDragBottom) {
      int diff =
          globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
      int newh = m_StartGeometry.height() + diff;
      if (newh > MIN_SIZE) {
        QRect newg = m_StartGeometry;
        newg.setHeight(newh);
        newg.setY(m_StartGeometry.y());
        setGeometry(newg);
      }
    }
  } else {
    // no mouse pressed
    if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
      setCursor(Qt::SizeFDiagCursor);
    } else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
      setCursor(Qt::SizeBDiagCursor);
    } else if (leftBorderHit(globalMousePos) &&
               bottomBorderHit(globalMousePos)) {
      setCursor(Qt::SizeBDiagCursor);
    } else {
      if (topBorderHit(globalMousePos)) {
        setCursor(Qt::SizeVerCursor);
      } else if (leftBorderHit(globalMousePos)) {
        setCursor(Qt::SizeHorCursor);
      } else if (rightBorderHit(globalMousePos)) {
        setCursor(Qt::SizeHorCursor);
      } else if (bottomBorderHit(globalMousePos)) {
        setCursor(Qt::SizeVerCursor);
      } else {
        m_bDragTop = false;
        m_bDragLeft = false;
        m_bDragRight = false;
        m_bDragBottom = false;
        setCursor(Qt::ArrowCursor);
      }
    }
  }
}

// pos in global virtual desktop coordinates
bool FramelessWindow::leftBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  if (pos.x() >= rect.x() && pos.x() <= rect.x() + CONST_DRAG_BORDER_SIZE) {
    return true;
  }
  return false;
}

bool FramelessWindow::rightBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  int tmp = rect.x() + rect.width();
  if (pos.x() <= tmp && pos.x() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
    return true;
  }
  return false;
}

bool FramelessWindow::topBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  if (pos.y() >= rect.y() && pos.y() <= rect.y() + CONST_DRAG_BORDER_SIZE) {
    return true;
  }
  return false;
}

bool FramelessWindow::bottomBorderHit(const QPoint &pos) {
  const QRect &rect = this->geometry();
  int tmp = rect.y() + rect.height();
  if (pos.y() <= tmp && pos.y() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
    return true;
  }
  return false;
}

void FramelessWindow::mousePressEvent(QMouseEvent *event) {
  if (isMaximized()) {
    return;
  }

  m_bMousePressed = true;
  m_StartGeometry = this->geometry();

  QPoint globalMousePos = mapToGlobal(event->position().toPoint());

  if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
    m_bDragTop = true;
    m_bDragLeft = true;
    setCursor(Qt::SizeFDiagCursor);
  } else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos)) {
    m_bDragRight = true;
    m_bDragTop = true;
    setCursor(Qt::SizeBDiagCursor);
  } else if (leftBorderHit(globalMousePos) && bottomBorderHit(globalMousePos)) {
    m_bDragLeft = true;
    m_bDragBottom = true;
    setCursor(Qt::SizeBDiagCursor);
  } else {
    if (topBorderHit(globalMousePos)) {
      m_bDragTop = true;
      setCursor(Qt::SizeVerCursor);
    } else if (leftBorderHit(globalMousePos)) {
      m_bDragLeft = true;
      setCursor(Qt::SizeHorCursor);
    } else if (rightBorderHit(globalMousePos)) {
      m_bDragRight = true;
      setCursor(Qt::SizeHorCursor);
    } else if (bottomBorderHit(globalMousePos)) {
      m_bDragBottom = true;
      setCursor(Qt::SizeVerCursor);
    }
  }
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  if (isMaximized()) {
    return;
  }

  m_bMousePressed = false;
  bool bSwitchBackCursorNeeded =
      m_bDragTop || m_bDragLeft || m_bDragRight || m_bDragBottom;
  m_bDragTop = false;
  m_bDragLeft = false;
  m_bDragRight = false;
  m_bDragBottom = false;
  if (bSwitchBackCursorNeeded) {
    setCursor(Qt::ArrowCursor);
  }
}

bool FramelessWindow::eventFilter(QObject *obj, QEvent *event) {
  QListView *listView = qobject_cast<QListView *>(obj);
  if (listView != nullptr) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (event->type() == QEvent::KeyPress &&
        keyEvent->key() == Qt::Key_Return) {
      //      if (obj->parent() != nullptr) {
      //        BoxPopupMenu *menu =
      //            qobject_cast<BoxPopupMenu *>(obj->parent()->parent());
      //        if (menu != nullptr) {
      //          emit menu->lineEdit()->returnPressed();
      //        }
      if (obj->parent() != nullptr) {
        LOG(logINFO) << "eventFilter"
                     << obj->parent()->metaObject()->className();
        BoxPopupMenu *menu =
            qobject_cast<BoxPopupMenu *>(obj->parent()->parent());
        if (menu != nullptr && menu->toolTip() == "pattern") {
          if (menu->index == 0) {
            QString text = menu->currentText();
            if (!text.isEmpty()) {
              text = text + "|" + menu->hilighted;
            } else {
              text = menu->hilighted;
            }
            menu->setCurrentText(text);
            emit menu->lineEdit()->returnPressed();
          } else if (menu->index == 1) {
            QString text = menu->mainMenu->currentText();
            if (!text.isEmpty()) {
              text = text + "|" + menu->hilighted;
            } else {
              text = menu->hilighted;
            }
            menu->mainMenu->setCurrentText(text);
            if (!menu->marked.isEmpty()) {
              emit addToQuickMark(menu->marked);
            }
            emit menu->mainMenu->lineEdit()->returnPressed();
            menu->hide();
          }
          menu->hidePopup();
        }
      }
    } else if (event->type() == QEvent::KeyPress &&
               (keyEvent->modifiers() & Qt::ControlModifier)) {
      QModelIndex currentIndex = listView->currentIndex();
      int count = listView->model()->rowCount();
      switch (keyEvent->key()) {
        case Qt::Key_P: {
          LOG(logINFO) << "eventFilter" << obj->metaObject()->className();
          PopListView *listView = qobject_cast<PopListView *>(obj);
          if (listView != nullptr) {
            QString text = listView->menu->mainMenu->currentText();
            if (!text.isEmpty()) {
              text = text + "|" + listView->menu->hilighted;
            } else {
              text = listView->menu->hilighted;
            }
            listView->menu->mainMenu->setCurrentText(text);
            if (!listView->menu->marked.isEmpty()) {
              emit addToQuickMark(listView->menu->marked);
            }
            emit listView->menu->mainMenu->lineEdit()->returnPressed();
            BoxPopupMenu *menu = listView->menu;
            menu->hidePopup();
            listView->hide();
            if (menu->index == 1) {
              menu->hide();
            }
            break;
          }

          if (obj->parent() == nullptr) {
            break;
          }
          LOG(logINFO) << "eventFilter"
                       << obj->parent()->metaObject()->className();
          BoxPopupMenu *menu =
              qobject_cast<BoxPopupMenu *>(obj->parent()->parent());
          if (menu != nullptr) {
            if (menu->index == 0) {
              QString text = menu->currentText();
              if (!text.isEmpty()) {
                text = text + "|" + menu->hilighted;
              } else {
                text = menu->hilighted;
              }
              menu->setCurrentText(text);
              emit menu->lineEdit()->returnPressed();
            } else if (menu->index == 1) {
              QString text = menu->mainMenu->currentText();
              if (!text.isEmpty()) {
                text = text + "|" + menu->hilighted;
              } else {
                text = menu->hilighted;
              }
              menu->mainMenu->setCurrentText(text);
              if (!menu->marked.isEmpty()) {
                emit addToQuickMark(menu->marked);
              }
              emit menu->mainMenu->lineEdit()->returnPressed();
              menu->hide();
            }
            menu->hidePopup();
          }
          break;
        }
        case Qt::Key_J: {
          int row = currentIndex.row() + 1;
          if (row >= count) {
            row = 0;
          }
          QModelIndex index = listView->model()->index(row, 0);
          listView->setCurrentIndex(index);
          break;
        }
        case Qt::Key_K: {
          int row = currentIndex.row() - 1;
          if (row < 0) {
            row = count - 1;
          }
          QModelIndex index = listView->model()->index(row, 0);
          listView->setCurrentIndex(index);
          break;
        }
      }
      return false;
    }
  }

  if (isMaximized()) {
    return QWidget::eventFilter(obj, event);
  }

  // check mouse move event when mouse is moved on any object
  if (event->type() == QEvent::MouseMove) {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse) {
      checkBorderDragging(pMouse);
    }
  }
  // press is triggered only on frame window
  else if (event->type() == QEvent::MouseButtonPress && obj == this) {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse) {
      mousePressEvent(pMouse);
    }
  } else if (event->type() == QEvent::MouseButtonRelease) {
    if (m_bMousePressed) {
      QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
      if (pMouse) {
        mouseReleaseEvent(pMouse);
      }
    }
  }

  return QWidget::eventFilter(obj, event);
}
