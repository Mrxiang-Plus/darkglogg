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

class sharedfilter : public QDialog
{
    Q_OBJECT
public:
    sharedfilter(QWidget *parent = nullptr);

protected:
    void addMainLayout();
    void addEditTabBtn();
    void addFilterTitle(int tabIndex);
    void addEditFilterBtn(int tabIndex);



private slots:
    void addTab_click();
    void delTab_click();
    void addFilterItem_click();
    void delFilterItem_click();

signals:
    void optionsChanged();

private:
    int tabCount;
    int filterCount;
    QTabWidget *mainTabWidget;

    QWidget *editTabHLayoutWidget;
    QWidget *editFilterHLayoutWidget;
    QWidget *filterItemHLayoutWidget;
    QWidget *filterTab;

    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;

    QPushButton *addTab;
    QPushButton *delTab;
    QPushButton *addFilterItem;
    QPushButton *delFilterItem;

    QVBoxLayout *mainVLayout;
    QHBoxLayout *editTabHLayout;
    QHBoxLayout *editFilterHLayout;
    QVBoxLayout *mainFilterVLayout;
    QHBoxLayout *filterTitleHLayout;
    QHBoxLayout *filterItemHLayout;




    QSpacerItem *horizontalSpacer;

};



#endif // SHAREDFILTER_H
