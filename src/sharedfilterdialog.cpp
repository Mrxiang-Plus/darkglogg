#include "sharedfilterdialog.h"
#include "ui_sharedfilterdialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabWidget>


sharedfilterdialog::sharedfilterdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sharedfilterdialog)
{
    ui->setupUi(this);
    addFilterTab();

    filterCount = 0;
    createFilterHead();
}

sharedfilterdialog::~sharedfilterdialog()
{
    delete ui;
}

void sharedfilterdialog::addFilterTab()
{
    filterTabTemp = new QWidget();
    filterTabTemp->setObjectName(QStringLiteral("filterTabTemp"));
    ui->sharedFilterWidget->addTab(filterTabTemp, QString());
    ui->sharedFilterWidget->setTabText(2, QApplication::translate("sharedfilterdialog", "App", Q_NULLPTR));
}

void sharedfilterdialog::createFilterHead()
{
    filterScroll = new QScrollArea(filterTabTemp);
    filterScroll->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    filterScroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    filterScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    filterScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    filterScroll->setGeometry(5, 5, 1200, 700);
    filterScroll->setContentsMargins(5, 5, 5, 5);

    mainWidget = new QWidget();
    mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setContentsMargins(0, 0, 0, 0);


    QHBoxLayout *hLayoutHead = new QHBoxLayout();
    hLayoutHead->setSpacing(10);
    hLayoutHead->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    hLayoutHead->setContentsMargins(0, 0, 0, 0);

    QLabel *keyLable = new QLabel("Key");
    keyLable->setFixedHeight(30);
    QLabel *filterLabel = new QLabel("Filter");
    filterLabel->setFixedHeight(30);
    QLabel *commentLabel = new QLabel("Comment");
    commentLabel->setFixedHeight(30);
    hLayoutHead->addWidget(keyLable, 1);
    hLayoutHead->addWidget(filterLabel, 4);
    hLayoutHead->addWidget(commentLabel, 2);


    mainLayout->addLayout(hLayoutHead);
    mainLayout->setSpacing(10);

    mainWidget->setLayout(mainLayout);
    filterScroll->setWidget(mainWidget);
    filterScroll->widget()->resize(QSize(1150, 50));

}

void sharedfilterdialog::on_addFilterItem_clicked()
{
    QLineEdit *keyEdit = new QLineEdit();
    keyEdit->setFixedHeight(25);
    QLineEdit *filterEdit = new QLineEdit();
    filterEdit->setFixedHeight(25);
    QLineEdit *commentEdit = new QLineEdit();
    commentEdit->setFixedHeight(25);
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->setSpacing(10);
    hLayout1->addWidget(keyEdit, 1);
    hLayout1->addWidget(filterEdit, 4);
    hLayout1->addWidget(commentEdit, 2);

    filterCount++;
    filterScroll->widget()->resize(QSize(1150, 50 + 35 * filterCount));
    mainLayout->addLayout(hLayout1);
}

void sharedfilterdialog::on_delFilterItem_clicked()
{

}
