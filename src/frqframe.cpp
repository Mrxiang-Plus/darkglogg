#include "frqframe.h"
#include "ui_frqframe.h"

FrqFrame::FrqFrame(QWidget *parent) : QWidget(parent), ui(new Ui::FrqFrame) {
  setupUi(this);
}

FrqFrame::~FrqFrame() { delete ui; }
