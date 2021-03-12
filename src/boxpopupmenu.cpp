#include "boxpopupmenu.h"
#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>
#include "log.h"

BoxPopupMenu::BoxPopupMenu(QWidget *parent) : QComboBox(parent) {
  this->installEventFilter(this);
  connect(this, QOverload<const QString &>::of(&QComboBox::highlighted),
          [=](const QString &text) {
            QStringList list = text.split('>');
            if (list.length() <= 1) {
              hilighted = text;
            } else {
              hilighted = list[1].trimmed();
              if (list.length() >= 3) {
                marked = list[2].trimmed();
              } else {
                marked = "";
              }
            }
          });
}

bool BoxPopupMenu::eventFilter(QObject *obj, QEvent *e) {
  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
  int key = keyEvent->key();

  if (e->type() == QEvent::KeyPress &&
      (keyEvent->modifiers() & Qt::ControlModifier)) {
    switch (key) {
      case Qt::Key_U: {
        this->clearEditText();
        break;
      }
      case Qt::Key_J: {
        this->showPopup();
        break;
      }
    }
  }

  return QWidget::eventFilter(obj, e);
}
