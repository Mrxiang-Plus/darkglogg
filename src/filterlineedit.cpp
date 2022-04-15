#include "filterlineedit.h"

#include <QLineEdit>

void FilterLineEdit::focusInEvent(QFocusEvent *e)
{
       QPalette p=QPalette();
       p.setColor(QPalette::Base,Qt::green);    //QPalette::Base is valid for the editable input box. There are other types to view documents
       setPalette(p);

}

void FilterLineEdit::focusOutEvent(QFocusEvent *e)
{
       QPalette p1=QPalette();
       p1.setColor(QPalette::Base,Qt::white);
       setPalette(p1);
}

void FilterLineEdit::mousePressEvent(QMouseEvent *event)

{
    QString str = this->objectName();
    QStringList strList = str.split("_");
    index = strList[strList.size()-1].toInt();
    emit click(index);

    QLineEdit::mousePressEvent(event);
}



