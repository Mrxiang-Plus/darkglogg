#include "syncfilterdialog.h"

#include <QDialog>
#include "sharedfilterdialog.h"
#include <QStringList>
#include <QCheckBox>
#include <QString>

SyncFilterDialog::SyncFilterDialog(QWidget *parent)
    :QDialog(parent)
{

}

void SyncFilterDialog::built(QStringList tabNameList)
{
    this->setWindowTitle(tr("Sync Filters"));
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    optionLayout = new QVBoxLayout(this);
    foreach (QString tabName, tabNameList)
    {
        if (tabName.compare("glogg_pattern") != 0 && tabName.compare("invalid") != 0)
        {
            addTabOption(tabName);
        }
    }
    addBoxBotton();
}
void SyncFilterDialog::addTabOption(QString tabName)
{
    checkBox = new QCheckBox(tabName);
    optionLayout->addWidget(checkBox);
}

void SyncFilterDialog::addBoxBotton()
{
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonBox_clicked(QAbstractButton*)));

    optionLayout->addWidget(buttonBox);
}

void SyncFilterDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
    if (role == QDialogButtonBox::AcceptRole) {
        QObjectList objectList = this->children();
        bool needEnd = false;
        foreach (QObject *obj, objectList)
        {
            if(QString(obj->metaObject()->className()) == QString("QCheckBox")
                    && ((QCheckBox*) obj)->isChecked() == true
                    && !addedTabList.contains(((QCheckBox*) obj)->text()))
            {
                //sync the selected filter group
                emit sync_applied(((QCheckBox*) obj)->text());
                addedTabList << ((QCheckBox*) obj)->text();
                needEnd = true;
            }
        }
        if (needEnd)
        {
            //emit end flag to combine local and remote filters setting
            emit sync_applied("end");
        }
        accept();
    }
    else if (role == QDialogButtonBox::RejectRole)
    {
      reject();
    }
}

