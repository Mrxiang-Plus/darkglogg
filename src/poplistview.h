#ifndef POPLISTVIEW_H
#define POPLISTVIEW_H

#include <QListView>
#include "boxpopupmenu.h"

class PopListView : public QListView {
  Q_OBJECT

 public:
  PopListView(QWidget *parent = 0);
  BoxPopupMenu *menu;

 private:
  bool eventFilter(QObject *t, QEvent *e);
};

#endif
