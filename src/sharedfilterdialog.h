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
#include "filterlineedit.h"

class SharedFilterDialog : public QDialog
{
    Q_OBJECT
public:
    SharedFilterDialog(QWidget *parent = nullptr);
//    ~SharedFilterDialog();

protected:
    void addMainLayout();
    void addEditTabBtn();
    void addFilterTitle(int tabIndex);
    void addEditFilterBtn(int tabIndex);
    void insertTabVLayout(int tabIndex);
    void deletectItem(QLayout *layout);
    QStringList getFilterLineName(int index);
    void delFilterLineByName();


private slots:
    void addTab_click();
    void delTab_click();
    void addFilterItem_click();
    void delFilterItem_click();

//    void getFilterLineIndex(int index);
    void handleClick(int index);


signals:
    void optionsChanged();

private:
    int tabCount;
    int curFilterCount;
    int filterLineNameIndex;

    QStringList filterLineName;

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

};



#endif // SHAREDFILTER_H
