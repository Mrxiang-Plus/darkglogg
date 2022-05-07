#ifndef SYNCFILTERDIALOG_H
#define SYNCFILTERDIALOG_H

#include <QtWidgets/QDialog>
#include "sharedfilterdialog.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QStringList>
#include <QString>

class SyncFilterDialog : public QDialog
{
    Q_OBJECT
public:
    SyncFilterDialog(QWidget *parent = nullptr);
//    SyncFilterDialog(QWidget *parent = nullptr, QStringList tabNameList);
    void built(QStringList tabNameList);
protected:
    void addTabOption(QString tabName);
    void addBoxBotton();

private:
    QVBoxLayout* optionLayout;
    QCheckBox* checkBox;
    QDialogButtonBox* buttonBox;

    QStringList selectedTabList;
    QStringList addedTabList;

private slots:
    void buttonBox_clicked(QAbstractButton* button);

signals:
    void sync_applied(const QString &sync_tabName);
};




#endif // SYNCFILTERDIALOG_H

