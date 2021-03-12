#include "poplistview.h"
#include <QEvent>
#include <QKeyEvent>
#include "log.h"

PopListView::PopListView(QWidget *parent) : QListView(parent) {
  this->installEventFilter(this);
  // setAutoCompletion(true);
}

bool PopListView::eventFilter(QObject *obj, QEvent *e) {
  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
  int key = keyEvent->key();

  if (e->type() == QEvent::KeyPress) {
    LOG(logINFO) << "event filter";
  }
  if (e->type() == QEvent::KeyPress &&
      (keyEvent->modifiers() & Qt::ControlModifier)) {
    switch (key) {
      case Qt::Key_K:
        // case Qt::Key_Up:
        {
          QKeyEvent *k =
              new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
          // changes the item selection
          // QComboBox::keyPressEvent(keyEvent);
          QListView::keyPressEvent(k);

          break;
        }
    }
  }

  return QWidget::eventFilter(obj, e);
}
