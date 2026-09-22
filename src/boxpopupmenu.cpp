#include "boxpopupmenu.h"
#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>
#include "log.h"

BoxPopupMenu::BoxPopupMenu(QWidget *parent) : QComboBox(parent) {
  this->installEventFilter(this);
  // Qt 6 split the overloaded QComboBox::highlighted(QString) off into
  // textHighlighted(); the int-only version is all that is left of the name.
  connect(this, &QComboBox::textHighlighted,
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

  if (e->type() == QEvent::KeyPress &&
      (keyEvent->modifiers() & Qt::ControlModifier)) {
    switch (keyEvent->key()) {
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
