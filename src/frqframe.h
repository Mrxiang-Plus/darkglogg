#ifndef FRQFRAME_H
#define FRQFRAME_H

#include <QWidget>

#include "ui_frqframe.h"
class FrqFrame : public QWidget, public Ui::FrqFrame {
  Q_OBJECT

 public:
  explicit FrqFrame(QWidget *parent = nullptr);
  ~FrqFrame();

 private:
  Ui::FrqFrame *ui;
};

#endif  // FRQFRAME_H
