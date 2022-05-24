#ifndef SHAREDFILTER_H
#define SHAREDFILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialogButtonBox>
#include "filterlineedit.h"
#include "sharedfilterset.h"
#include "syncfilterdialog.h"

class SavedPatterns;

class SharedFilterDialog : public QDialog
{
    Q_OBJECT
public:
    SharedFilterDialog(QWidget *parent = nullptr);

protected:
    void addMainLayout();
    void addEditTabBtn();
    void addFilterTitle(int tabIndex);
    void addEditFilterBtn(int tabIndex);
    void insertTabVLayout(int tabIndex);
    void deletectItem(QLayout *layout);
    QStringList getFilterLineName(int index);
    void delFilterLineByName();
    //remove the sharedfilter from sharedFilterSet by key.
    void delFilterDataByKey(QString keyStr);
    void delFilterDataByTab(QString tabStr);
    void rebuildDialog();
    void rebuildTab(QString tabName);
    void rebuildFilter(int tabIndex, QString key, QString pattern, QString comment);
    void saveData();
    QString getRemoteDir(QString sshStr);
    QStringList getRemoteTabSet();
    QStringList getLocalTabSet();

    //sync filter from savedpatterns
    void importSavedPattern();
    virtual void doSetSavedPatterns(
        std::shared_ptr<SavedPatterns> saved_patterns);

private slots:
    void addTab_click();
    void delTab_click();
    void addFilterItem_click();
    void delFilterItem_click();
    void syncFilterGroup_click();

//    void getFilterLineIndex(int index);
    void handleClick(int index);
    void buttonBox_clicked(QAbstractButton* button);    
    void handleSyncApplied(const QString &sync_tabName);

signals:
    void optionsChanged();

private:
    int tabCount;
    int curFilterCount;
    int filterLineNameIndex;

    QStringList filterLineName;

    QStringList tabNameSet;
//    QStringList remoteTabSet;
//    QStringList localTabSet;

    //count filters contained in each tab.
    QVector<int> filterArray;


    QTabWidget *mainTabWidget;

    QWidget *editTabHLayoutWidget;
    QWidget *editFilterHLayoutWidget;
    QWidget *filterItemHLayoutWidget;
    QWidget *filterTab;
    QWidget *curTabWidget;

    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;

    QPushButton *addTab;
    QPushButton *delTab;
    QPushButton *addFilterItem;
    QPushButton *delFilterItem;
    QPushButton *syncFilterGroup;
    QDialogButtonBox* buttonBox;

    QVBoxLayout *mainVLayout;
    QVBoxLayout *tabVLayout;
    QHBoxLayout *editTabHLayout;
    QHBoxLayout *editFilterHLayout;
    QVBoxLayout *mainFilterVLayout;
    QHBoxLayout *filterTitleHLayout;
    QHBoxLayout *filterItemHLayout;

    FilterLineEdit *keyEdit;
    FilterLineEdit *filterEdit;
    FilterLineEdit *commentEdit;


    QSpacerItem *horizontalSpacer;

    std::shared_ptr<SharedFilterSet> sharedFilterSet;
    std::shared_ptr<SavedPatterns> savedPatterns_;

};



#endif // SHAREDFILTER_H
