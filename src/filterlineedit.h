#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H


#include <QLineEdit>
#include <qboxlayout.h>


class FilterLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    int index;

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);


signals:
    //send the last focus filteEditLine index
    void click(int index);
};

#endif // FILTERLINEEDIT_H
