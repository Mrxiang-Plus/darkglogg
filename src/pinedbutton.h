#ifndef PINEDBUTTON_H
#define PINEDBUTTON_H

#include <QMutex>
#include <QToolButton>

class PinedButton : public QToolButton {
  Q_OBJECT
 public:
  explicit PinedButton(QToolButton* parent = nullptr);
  void setPinedIndex(int index);
  int pinedIndex();
  void setColor(QColor backColor, QColor foreColor);

  bool getPressed();
  void setPressed(bool pressed);

 private:
  int pinedIndex_;
  QColor backColor_;
  QColor foreColor_;
  bool pressed_;
  QMutex mutex_;

 signals:
  void clicked(int);
  void removed(int);

 public slots:
  void onClick();
  void hightlightButton();
  void ContrastColor(QColor* color, QColor* foreColor);
};

#endif  // PINEDBUTTON_H
