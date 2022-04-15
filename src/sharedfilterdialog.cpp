#include "sharedfilterdialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>
#include "qdebug.h"

SharedFilterDialog::SharedFilterDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowTitle("Shared Filter");
    this->resize(1300, 800);
    addMainLayout();
    addFilterTitle(0);
    addEditFilterBtn(0);
    insertTabVLayout(0);
}

//               --mainTabWidget
//sharedfilter--|
//               --addTab--delTab
void SharedFilterDialog::addMainLayout()
{
    mainVLayout = new QVBoxLayout(this);
    mainTabWidget = new QTabWidget();
    mainTabWidget->setGeometry(QRect(10, 10, 1260, 720));

    filterTab = new QWidget(mainTabWidget);
    filterTab->setObjectName("App");
    mainTabWidget->addTab(filterTab, QString());
    mainTabWidget->setTabText(0, "Cam App");
    tabCount = 1;
    filterArray.append(0);
//    sharedFilterWidget->setTabText(sharedFilterWidget->indexOf(filterTab), QApplication::translate("sharedfilterdialog", "Cam_App", Q_NULLPTR));

    editTabHLayoutWidget = new QWidget();
    editTabHLayoutWidget->setFixedSize(300, 40);
    editTabHLayout = new QHBoxLayout(editTabHLayoutWidget);
    editTabHLayout->setAlignment(Qt::AlignCenter);
    addTab = new QPushButton();
    addTab->setText("New Tab");
    addTab->setFixedHeight(30);
    connect(addTab, SIGNAL(clicked()), this, SLOT(addTab_click()));
    delTab = new QPushButton();
    delTab->setText("Del Tab");
    delTab->setFixedHeight(30);
    connect(delTab, SIGNAL(clicked()), this, SLOT(delTab_click()));
    editTabHLayout->addWidget(addTab);
    editTabHLayout->addWidget(delTab);

    mainVLayout->addWidget(mainTabWidget);
    mainVLayout->addWidget(editTabHLayoutWidget);
}

void SharedFilterDialog::addFilterTitle(int tabIndex)
{
    QWidget *curTabWidget = mainTabWidget->widget(tabIndex);
    scrollArea = new QScrollArea();

    scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //710
    scrollArea->setGeometry(5, 5, 1200, 200);
    scrollArea->setContentsMargins(5, 5, 5, 5);

    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollwidget"));
    scrollAreaWidgetContents->resize(1200, 300);

    mainFilterVLayout = new QVBoxLayout();
    mainFilterVLayout->setObjectName(QStringLiteral("main_filter_v_layout"));
    mainFilterVLayout->setSpacing(10);
    mainFilterVLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mainFilterVLayout->setContentsMargins(0, 0, 0, 0);

    filterTitleHLayout = new QHBoxLayout();
    filterTitleHLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    filterTitleHLayout->setSpacing(10);
    filterTitleHLayout->setObjectName(QStringLiteral("filter_title_h_layout"));

    QLabel *keyLable = new QLabel("Key");
    keyLable->setFixedHeight(30);
    QLabel *filterLabel = new QLabel("Filter");
    filterLabel->setFixedHeight(30);
    QLabel *commentLabel = new QLabel("Comment");
    commentLabel->setFixedHeight(30);
    filterTitleHLayout->addWidget(keyLable, 1);
    filterTitleHLayout->addWidget(filterLabel, 4);
    filterTitleHLayout->addWidget(commentLabel, 2);

    mainFilterVLayout->addLayout(filterTitleHLayout);
    scrollAreaWidgetContents->setLayout(mainFilterVLayout);
    scrollArea->setWidget(scrollAreaWidgetContents);

}

void SharedFilterDialog::addEditFilterBtn(int tabIndex)
{
    QWidget *curTabWidget = mainTabWidget->widget(tabIndex);

    addFilterItem = new QPushButton();
    addFilterItem->setText("Add Filter");
    addFilterItem->setFixedHeight(30);
    addFilterItem->setFixedWidth(120);
    connect(addFilterItem, SIGNAL(clicked()), this, SLOT(addFilterItem_click()));

    delFilterItem = new QPushButton();
    delFilterItem->setText("Del Filter");
    delFilterItem->setFixedHeight(30);
    delFilterItem->setFixedWidth(120);
    connect(delFilterItem, SIGNAL(clicked()), this, SLOT(delFilterItem_click()));

    editFilterHLayoutWidget = new QWidget();
    editFilterHLayoutWidget->setFixedSize(300, 40);
    editFilterHLayout = new QHBoxLayout(editFilterHLayoutWidget);
    editFilterHLayout->setAlignment(Qt::AlignRight);
    editFilterHLayout->addWidget(addFilterItem, 2);
    editFilterHLayout->addStretch(1);
    editFilterHLayout->addWidget(delFilterItem, 2);

}

void SharedFilterDialog::insertTabVLayout(int tabIndex) {
    curTabWidget = mainTabWidget->widget(tabIndex);
    tabVLayout = new QVBoxLayout(curTabWidget);
    tabVLayout->addWidget(scrollArea);
    tabVLayout->addWidget(editFilterHLayoutWidget);
    tabVLayout->setAlignment(Qt::AlignTop|Qt::AlignRight);
}

void SharedFilterDialog::addTab_click()
{
    bool ok = false;
    QString tabText = QInputDialog :: getText(this,
                                             "Add Tab",
                                             "Please input the tab title",
                                             QLineEdit::Normal,
                                             "tab",
                                             &ok
                                             );
    if (ok && !tabText.isEmpty()) {
        QWidget *tempTab = new QWidget(mainTabWidget);
        mainTabWidget->addTab(tempTab, QString());
        mainTabWidget->setTabText(tabCount, tabText);
        addFilterTitle(tabCount);
        addEditFilterBtn(tabCount);
        insertTabVLayout(tabCount);
        tabCount++;
        filterArray.append(0);
    }

}

void SharedFilterDialog::delTab_click()
{
    bool ok = false;
    int curIndex = mainTabWidget->currentIndex();
    QString curTab = mainTabWidget->tabText(curIndex);
    QString tabText = QInputDialog :: getText(this,
                                             "Del Tab",
                                             "Please reconfirm the current tab name.",
                                             QLineEdit::Normal,
                                             "",
                                             &ok
                                             );
    if (ok && !tabText.isEmpty() && !tabText.compare(curTab))
    {
        curTabWidget = mainTabWidget->widget(curIndex);
        deletectItem(curTabWidget->layout());
        mainTabWidget->removeTab(curIndex);
        filterArray.remove(curIndex, 1);
        tabCount--;
    }
    else if (ok)
    {
        QMessageBox *msgBox = new QMessageBox("Warning",
                                              "Verification failed.",
                                              QMessageBox::Warning,
                                              QMessageBox::Ok,
                                              QMessageBox::Escape,
                                              0);
        msgBox->show();
    }
}

void SharedFilterDialog::deletectItem(QLayout* layout)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr)
    {
        if(child->widget())
        {
            child->widget()->setParent(nullptr);
            delete child->widget();
        }
        else if(child->layout())
        {
            deletectItem(child->layout());
            child->layout()->deleteLater();
        }


    }
}

void SharedFilterDialog::addFilterItem_click()
{
    int curIndex = mainTabWidget->currentIndex();
    curFilterCount = filterArray[curIndex];
    curTabWidget = mainTabWidget->widget(curIndex);
    scrollAreaWidgetContents = curTabWidget->findChild<QWidget *>("scrollwidget");
    mainFilterVLayout = scrollAreaWidgetContents->findChild<QVBoxLayout *>("main_filter_v_layout");
    bool ok = false;
    QString filterText = QInputDialog :: getText(this,
                                             "Add Filter",
                                             "Please input the filter item. \n Such as: key > filter > comment.\n Comment can be ignored.",
                                             QLineEdit::Normal,
                                             "",
                                             &ok
                                             );
    //parse filter
    QStringList filterList = filterText.split(">");
    //for scroll debug
    for (; curFilterCount < 20 ;) {
    if (ok && filterList.size() >= 2) {
        QLineEdit *keyEdit = new QLineEdit();
        keyEdit->setFixedHeight(25);
        keyEdit->setEnabled(true);
        keyEdit->setText(filterList.at(0));
        QLineEdit *filterEdit = new QLineEdit();
        filterEdit->setFixedHeight(25);
        filterEdit->setText(filterList.at(1));
        QLineEdit *commentEdit = new QLineEdit();
        commentEdit->setFixedHeight(25);
        if (filterList.size() > 2)
        {
            commentEdit->setText(filterList.at(2));
        }


        filterItemHLayout = new QHBoxLayout(filterItemHLayoutWidget);
        filterItemHLayout->setSpacing(10);
        filterItemHLayout->addWidget(keyEdit, 1);
        filterItemHLayout->addWidget(filterEdit, 4);
        filterItemHLayout->addWidget(commentEdit, 2);
        QString filterItemName = "filter_item#" +curFilterCount;
        filterItemHLayout->setObjectName("filter_item#" +curFilterCount);

        mainFilterVLayout->addLayout(filterItemHLayout);

        curFilterCount++;
        filterArray.replace(curIndex, curFilterCount);
        scrollAreaWidgetContents->resize(1160, 50 + 35 * curFilterCount);
    }
    }
}

void SharedFilterDialog::delFilterItem_click()
{
    int curIndex = mainTabWidget->currentIndex();
    curFilterCount = filterArray[curIndex];
    curTabWidget = mainTabWidget->widget(curIndex);
    scrollAreaWidgetContents = curTabWidget->findChild<QWidget *>("scrollwidget");
    mainFilterVLayout = scrollAreaWidgetContents->findChild<QVBoxLayout *>("main_filter_v_layout");

    // todo: how to get the focus filterItemHLayout and delete.
    mainFilterVLayout->removeItem(filterItemHLayout);

}
