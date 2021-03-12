#include "pinedbutton.h"

#include <QGraphicsDropShadowEffect>

PinedButton::PinedButton(QToolButton *parent) : QToolButton(parent) {
  pressed_ = false;
}

void PinedButton::setPinedIndex(int index) { pinedIndex_ = index; }
int PinedButton::pinedIndex() { return pinedIndex_; }

void PinedButton::setColor(QColor backColor, QColor foreColor) {
  backColor_ = backColor;
  foreColor_ = foreColor;
}

bool PinedButton::getPressed() { return pressed_; }

void PinedButton::setPressed(bool pressed) {
  mutex_.lock();
  pressed_ = pressed;
  mutex_.unlock();
}

void PinedButton::onClick() {
  mutex_.lock();
  pressed_ = !pressed_;
  mutex_.unlock();
  if (pressed_) {
    setStyleSheet(QString("color :rgb(%1,%2,%3);"
                          "background-color: rgb(%4,%5,%6);")
                      .arg(foreColor_.red())
                      .arg(foreColor_.green())
                      .arg(foreColor_.blue())
                      .arg(backColor_.red())
                      .arg(backColor_.green())
                      .arg(backColor_.blue()));
    emit clicked(pinedIndex_);

  } else {
    setStyleSheet("");
    emit removed(pinedIndex_);
  }
}

void PinedButton::ContrastColor(QColor *color, QColor *foreColor) {
  int d = 0;

  // Counting the perceptive luminance - human eye favors green color...
  double luminance =
      (0.299 * color->red() + 0.587 * color->green() + 0.114 * color->blue()) /
      255;

  if (luminance > 0.5)
    d = 0;  // bright colors - black font
  else
    d = 255;  // dark colors - white font

  foreColor->setRgb(d, d, d);
}
