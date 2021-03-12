#ifndef BOXPOPUPMENU_H
#define BOXPOPUPMENU_H

#include <QComboBox>

class BoxPopupMenu : public QComboBox {
  Q_OBJECT

 public:
  BoxPopupMenu(QWidget *parent = 0);
  QString hilighted;
  QString marked;
  int index;
  BoxPopupMenu *mainMenu;

 private:
  bool eventFilter(QObject *t, QEvent *e);
};

#endif
