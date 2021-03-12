/*
 * Copyright (C) 2014, 2015 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tabbedcrawlerwidget.h"

#include <QKeyEvent>
#include <QLabel>

#include "crawlerwidget.h"

#include <QPainter>
#include <QStyleOption>
#include "log.h"

TabbedCrawlerWidget::TabbedCrawlerWidget(QWidget *parent)
    : QTabWidget(parent),
      olddata_icon_(":/images/olddata_icon.png"),
      newdata_icon_(":/images/newdata_icon.png"),
      newfiltered_icon_(":/images/newfiltered_icon.png"),
      myTabBar_() {
  setTabBar(&myTabBar_);
  mousePressed = false;
}

// I know hiding non-virtual functions from the base class is bad form
// and I do it here out of pure laziness: I don't want to encapsulate
// QTabBar with all signals and all just to implement this very simple logic.
// Maybe one day that should be done better...

int TabbedCrawlerWidget::addTab(QWidget *page, const QString &label,
                                const QString &filePath) {
  int index = QTabWidget::addTab(page, label);

  if (auto crawler = dynamic_cast<CrawlerWidget *>(page)) {
    // Mmmmhhhh... new Qt5 signal syntax create tight coupling between
    // us and the sender, baaaaad....

    // Listen for a changing data status:
    connect(crawler, &CrawlerWidget::dataStatusChanged,
            [this, filePath](DataStatus status) {
              setTabDataStatus(filePath, status);
            });
  }

  // Display the icon
  QLabel *icon_label = new QLabel();
  icon_label->setPixmap(olddata_icon_.pixmap(11, 12));
  icon_label->setAlignment(Qt::AlignCenter);
  myTabBar_.setTabButton(index, QTabBar::LeftSide, icon_label);

  LOG(logDEBUG) << "addTab, count = " << count();
  LOG(logDEBUG) << "width = "
                << olddata_icon_.pixmap(11, 12).devicePixelRatio();

  if (count() > 0) myTabBar_.show();

  setTabToolTip(index, filePath);
  return index;
}

void TabbedCrawlerWidget::removeTab(int index) {
  QTabWidget::removeTab(index);

  if (count() <= 1) myTabBar_.hide();
}

void TabbedCrawlerWidget::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  mousePressed = false;
  LOG(logDEBUG) << "TabbedCrawlerWidget::mouseReleaseEvent";

  if (event->button() == Qt::MidButton) {
    int tab = this->myTabBar_.tabAt(event->pos());
    if (-1 != tab) {
      emit tabCloseRequested(tab);
    }
  }
}

void TabbedCrawlerWidget::keyPressEvent(QKeyEvent *event) {
  const auto mod = event->modifiers();
  const auto key = event->key();

  LOG(logDEBUG) << "TabbedCrawlerWidget::keyPressEvent";

  // Ctrl + tab
  if ((mod == Qt::ControlModifier && key == Qt::Key_Tab) ||
      (mod == (Qt::ControlModifier | Qt::AltModifier | Qt::KeypadModifier) &&
       key == Qt::Key_Right)) {
    setCurrentIndex((currentIndex() + 1) % count());
  }
  // Ctrl + shift + tab
  else if ((mod == (Qt::ControlModifier | Qt::ShiftModifier) &&
            key == Qt::Key_Tab) ||
           (mod ==
                (Qt::ControlModifier | Qt::AltModifier | Qt::KeypadModifier) &&
            key == Qt::Key_Left)) {
    setCurrentIndex((currentIndex() - 1 >= 0) ? currentIndex() - 1
                                              : count() - 1);
  }
  // Ctrl + numbers
  else if (mod == Qt::ControlModifier &&
           (key >= Qt::Key_1 && key <= Qt::Key_8)) {
    int new_index = key - Qt::Key_0;
    if (new_index <= count()) setCurrentIndex(new_index - 1);
  }
  // Ctrl + 9
  else if (mod == Qt::ControlModifier && key == Qt::Key_9) {
    setCurrentIndex(count() - 1);
  } else if (mod == Qt::ControlModifier && key == Qt::Key_W) {
    emit tabCloseRequested(currentIndex());
  } else {
    QTabWidget::keyPressEvent(event);
  }
}

void TabbedCrawlerWidget::setTabDataStatus(const QString &filePath,
                                           DataStatus status) {
  LOG(logDEBUG) << "TabbedCrawlerWidget::setTabDataStatus ";
  QString s;
  int index = 0;
  for (int i = 0; i < count(); i++) {
    s = tabToolTip(i);
    if (s == filePath) {
      index = i;
    }
  }
  QLabel *icon_label =
      dynamic_cast<QLabel *>(myTabBar_.tabButton(index, QTabBar::LeftSide));

  if (icon_label) {
    const QIcon *icon;
    switch (status) {
      case DataStatus::OLD_DATA:
        icon = &olddata_icon_;
        break;
      case DataStatus::NEW_DATA:
        icon = &newdata_icon_;
        break;
      case DataStatus::NEW_FILTERED_DATA:
        icon = &newfiltered_icon_;
        break;
      default:
        return;
    }

    icon_label->setPixmap(icon->pixmap(12, 12));
  }
}

void TabbedCrawlerWidget::setTabBarVisibility(bool visible) {
  if (visible) {
    myTabBar_.show();
  } else {
    myTabBar_.hide();
  }
}

bool TabbedCrawlerWidget::getTabBarVisibility() {
  return myTabBar_.isVisible();
}
void TabbedCrawlerWidget::mousePressEvent(QMouseEvent *event) {
  mousePressed = true;
  mousePos = event->globalPos();

  QWidget *parent = parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();

  if (parent) wndPos = parent->pos();
}

void TabbedCrawlerWidget::mouseMoveEvent(QMouseEvent *event) {
  QWidget *parent = parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();

  if (parent && mousePressed)
    parent->move(wndPos + (event->globalPos() - mousePos));
}

void TabbedCrawlerWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QStyleOption styleOption;
  styleOption.init(this);
  QPainter painter(this);
  style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
}

void TabbedCrawlerWidget::mouseDoubleClickEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  QWidget *parent = parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) parent = parent->parentWidget();
  if (parent) {
    if (parent->windowState().testFlag(Qt::WindowNoState)) {
      parent->setWindowState(Qt::WindowMaximized);
    } else if (parent->windowState().testFlag(Qt::WindowMaximized)) {
      parent->setWindowState(Qt::WindowNoState);
      parent->hide();
      parent->show();
    }
  }
}
