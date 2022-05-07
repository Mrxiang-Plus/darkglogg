#include "sharedfilterdialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "filterlineedit.h"
#include "qdebug.h"
#include "persistentinfo.h"
#include "configuration.h"
#include "log.h"
#include "savedpatterns.h"
#include "persistentpattern.h"
#include "syncfilterdialog.h"

#include <QDir>
#include <QProcess>
#include <QTimer>

SharedFilterDialog::SharedFilterDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowTitle("Shared Filter");
    this->resize(1300, 800);
    addMainLayout();
//    addFilterTitle(0);
//    addEditFilterBtn(0);
//    insertTabVLayout(0);

    // Reload the filter list from disk (in case it has been changed
    // by another glogg instance) and copy it to here.
//    importSavedPattern();
    GetPersistentPattern().retrieve("sharedFilterSet");
    sharedFilterSet = PatternPersistentCopy<SharedFilterSet>("sharedFilterSet");
    if (!sharedFilterSet->sharedFilterList.empty())
    {
        rebuildDialog();
    }
}

//               --mainTabWidget
//sharedfilter--|
//               --addTab--delTab
void SharedFilterDialog::addMainLayout()
{
    mainVLayout = new QVBoxLayout(this);
    mainTabWidget = new QTabWidget();
    mainTabWidget->setGeometry(QRect(10, 10, 1260, 720));

//    filterTab = new QWidget(mainTabWidget);
//    filterTab->setObjectName("App");
//    mainTabWidget->addTab(filterTab, QString());
//    mainTabWidget->setTabText(0, "Cam App");
//    filterArray.append(0);
//    tabCount = 1;

    tabCount = 0;

    editTabHLayoutWidget = new QWidget();
    editTabHLayoutWidget->setFixedHeight( 40);
    editTabHLayout = new QHBoxLayout(editTabHLayoutWidget);
    editTabHLayout->setAlignment(Qt::AlignCenter);
    addTab = new QPushButton();
    addTab->setText("New Tab");
    addTab->setFixedHeight(30);
    addTab->setFixedWidth(90);
    connect(addTab, SIGNAL(clicked()), this, SLOT(addTab_click()));
    delTab = new QPushButton();
    delTab->setText("Del Tab");
    delTab->setFixedHeight(30);
    delTab->setFixedWidth(90);
    connect(delTab, SIGNAL(clicked()), this, SLOT(delTab_click()));

    syncFilterGroup = new QPushButton();
    syncFilterGroup->setText("Sync");
    syncFilterGroup->setFixedHeight(30);
    syncFilterGroup->setFixedWidth(90);
    connect(syncFilterGroup, SIGNAL(clicked()), this, SLOT(syncFilterGroup_click()));

    editTabHLayout->addWidget(addTab);
    editTabHLayout->addWidget(delTab);
    editTabHLayout->addWidget(syncFilterGroup);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                      | QDialogButtonBox::Apply
                                                      | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonBox_clicked(QAbstractButton*)));

    mainVLayout->addWidget(mainTabWidget);
    mainVLayout->addWidget(editTabHLayoutWidget);
    mainVLayout->addWidget(buttonBox);
    mainTabWidget->setStyleSheet("QTabWidget:pane{ \
                        border: 2px solid darkgray;top: -1px;background-color:transparent;}\
                        QTabBar::tab{height:22px; background-color:black; margin-right: 2px; margin-bottom:-2px;}\
                        QTabBar::tab:selected{border:2px solid darkgray;border-bottom-color: none;background:rgb(33,33,33);color:white;}\
                        QTabBar::tab:!selected{background:rgb(80,78,71);color:white;}\
                        ");
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
    editFilterHLayoutWidget->setFixedHeight(40);

    editFilterHLayout = new QHBoxLayout(editFilterHLayoutWidget);
    editFilterHLayout->setAlignment(Qt::AlignRight);
    editFilterHLayout->addWidget(addFilterItem, 1);
    editFilterHLayout->addWidget(delFilterItem, 1);

}

void SharedFilterDialog::insertTabVLayout(int tabIndex)
{
    curTabWidget = mainTabWidget->widget(tabIndex);
    tabVLayout = new QVBoxLayout(curTabWidget);
    tabVLayout->addWidget(scrollArea);
    tabVLayout->addWidget(editFilterHLayoutWidget);
    tabVLayout->setAlignment(Qt::AlignTop|Qt::AlignRight);
}

void SharedFilterDialog::doSetSavedPatterns(
    std::shared_ptr<SavedPatterns> saved_patterns) {
  savedPatterns_ = saved_patterns;
}

void SharedFilterDialog::importSavedPattern()
{
    GetPersistentPattern().retrieve(QString("savedPatterns"));
    savedPatterns_ = PatternPersistent<SavedPatterns>("savedPatterns");
    QListIterator<QString> itr(savedPatterns_->recentPatterns());
    GetPersistentPattern().retrieve("sharedFilterSet");
    sharedFilterSet = PatternPersistentCopy<SharedFilterSet>("sharedFilterSet");
    while (itr.hasNext())
    {
        QStringList filterList = itr.next().split(">");
        if (filterList.size() >= 2) {
            SharedFilter newFilter = SharedFilter("Cam-APP", filterList.at(0), filterList.at(1), "");
            if (filterList.size() > 2)
            {
                newFilter.setComment(filterList.at(2));
            }
            newFilter.setFilterItem();
            sharedFilterSet->sharedFilterList << newFilter;
        }
    }
    *(PatternPersistent<SharedFilterSet>("sharedFilterSet")) = *sharedFilterSet;
    GetPersistentPattern().save("sharedFilterSet");
    emit optionsChanged();
}

// restore the sharedFilterDialog from sharedFilterSet
void SharedFilterDialog::rebuildDialog()
{
    QList<SharedFilter> filterList = sharedFilterSet->sharedFilterList;
    tabNameSet.clear();
    foreach (SharedFilter sharedFilter, filterList)
    {
        sharedFilter.retrieveFromFilterItem();
        QString tabName = sharedFilter.tab();
        if (! tabNameSet.contains(tabName)) {
            rebuildTab(tabName);
            tabNameSet << tabName;
        }
        rebuildFilter(tabNameSet.indexOf(tabName), sharedFilter.key(), sharedFilter.pattern(), sharedFilter.comment());
    }
}

QStringList SharedFilterDialog::getLocalTabSet()
{
    QList<SharedFilter> filterList = sharedFilterSet->sharedFilterList;
    QStringList tabSet;
    int size = filterList.size();
    foreach (SharedFilter sharedFilter, filterList)
    {
        sharedFilter.retrieveFromFilterItem();
        QString tabName = sharedFilter.tab();
        if (! tabSet.contains(tabName)) {
            tabSet << tabName;
        }
    }
    return tabSet;
}

QStringList SharedFilterDialog::getRemoteTabSet()
{
    //  QProcess process;
    QProcess* process = new QProcess();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
    QString repoUrl = config->repoUrl();

    //clone the git@git.n.xiaomi.com:MiuiCamera/miuicameratool.git
  #ifdef _WIN32
    process->setWorkingDirectory(path);
    QString command = path + "sync-filter-group.bat" + sync_tabName;
    process->startDetached(command);
  #else

    QObject::connect(
        process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
          process->deleteLater();
        });
    //for sync sharedFilter from remote
    process->start("/bin/bash", QStringList() << path + "sync-filter-group.sh"
                                              << repoUrl);

  #endif

    //get the remote filter group by getting basename of .txt file.
    QDir remoteDir(path + "miuicameratool" + QDir::separator() + "glogg");
    QStringList filtername;
    filtername << "*.txt";
    remoteDir.setNameFilters(filtername);
    QStringList fileList = remoteDir.entryList();
    QFileInfoList fileInfoList = remoteDir.entryInfoList();
    QStringList ret;
    foreach (QFileInfo info, fileInfoList) {
        ret << info.baseName();
    }
    return ret;
}

void SharedFilterDialog::rebuildTab(QString tabName)
{
    QWidget *tempTab = new QWidget(mainTabWidget);
    mainTabWidget->addTab(tempTab, QString());

    mainTabWidget->setTabText(tabCount, tabName);
    addFilterTitle(tabCount);
    addEditFilterBtn(tabCount);
    insertTabVLayout(tabCount);
    mainTabWidget->setCurrentIndex(tabCount);
    tabCount++;
    filterArray.append(0);
}

void SharedFilterDialog::rebuildFilter(int tabIndex, QString key, QString pattern, QString comment)
{
    int curIndex = mainTabWidget->currentIndex();
    curFilterCount = filterArray[curIndex];
    QString curTabText = mainTabWidget->tabText(curIndex);
    curTabWidget = mainTabWidget->widget(curIndex);
    scrollAreaWidgetContents = curTabWidget->findChild<QWidget *>("scrollwidget");
    mainFilterVLayout = scrollAreaWidgetContents->findChild<QVBoxLayout *>("main_filter_v_layout");


    keyEdit = new FilterLineEdit();
    keyEdit->setFixedHeight(25);
    keyEdit->setText(key);
    filterEdit = new FilterLineEdit();
    filterEdit->setFixedHeight(25);
    filterEdit->setText(pattern);
    commentEdit = new FilterLineEdit();
    commentEdit->setFixedHeight(25);
    if (comment != NULL)
    {
        commentEdit->setText(comment);
    }

    QString indexStr = QString::number(curFilterCount);
    filterLineName.clear();
    filterLineName << ("filter_Line_" + indexStr) << ("filter_key_" + indexStr)
                   << ("filter_content_" + indexStr) << ("filter_comment_" + indexStr);
    keyEdit->setObjectName(filterLineName[1]);
    filterEdit->setObjectName(filterLineName[2]);
    commentEdit->setObjectName(filterLineName[3]);

    connect(keyEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));
    connect(filterEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));
    connect(commentEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));

    filterItemHLayout = new QHBoxLayout();
    filterItemHLayout->setObjectName(filterLineName[0]);
    filterItemHLayout->setSpacing(10);
    filterItemHLayout->addWidget(keyEdit, 1);
    filterItemHLayout->addWidget(filterEdit, 4);
    filterItemHLayout->addWidget(commentEdit, 2);

    mainFilterVLayout->addLayout(filterItemHLayout);

    curFilterCount++;
    filterArray.replace(curIndex, curFilterCount);
    scrollAreaWidgetContents->resize(1160, 50 + 35 * curFilterCount);
}

void SharedFilterDialog::syncFilterGroup_click()
{
    //apply settings before sync
    GetPersistentPattern().migrateAndInit("sharedFilterSet");
    *(PatternPersistent<SharedFilterSet>("sharedFilterSet")) = *sharedFilterSet;
    GetPersistentPattern().save("sharedFilterSet");
    emit optionsChanged();

    SyncFilterDialog syncDialog(this);
    // combine remote and local group.
    QStringList remoteTabSet = getRemoteTabSet();
    QStringList localTabSet = getLocalTabSet();
    foreach (QString localStr, localTabSet)
    {
        if (!remoteTabSet.contains(localStr))
        {
            remoteTabSet << localStr;
        }
    }
    //show sync dialog
    syncDialog.built(remoteTabSet);  

    connect(&syncDialog, SIGNAL(sync_applied(const QString &)), this, SLOT(handleSyncApplied(const QString &)));
    syncDialog.exec();
}

void SharedFilterDialog::handleSyncApplied(const QString &sync_tabName)
{

    QProcess* process = new QProcess();
    QProcess* process1 = new QProcess();

    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

    std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
    QString repoUrl = config->repoUrl();


  #ifdef _WIN32
    process->setWorkingDirectory(path);
    QString command = path + "sync-filter-group.bat" + sync_tabName;
    process->startDetached(command);
  #else

    // catch data output
    // QObject::connect(process, &QProcess::readyRead, [process]() {
    // QByteArray a = process->readAll();
    // qDebug() << a;
    // });

    // delete process instance when done, and get the exit status to handle
    // errors.
    QObject::connect(
        process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
//          emit refreshPatterns();
          process->deleteLater();
        });
    if (sync_tabName.compare("end") != 0)
    {
        //for sync sharedFilter from remote
        process->start("/bin/bash", QStringList() << path + "sync-filter-group.sh"
                                                  << repoUrl
                                                  << sync_tabName);
    }
    //for recover local sharedFilter setting
    process1->start("/bin/bash", QStringList() << path + "update_filter_setting.sh"
                                              << repoUrl
                                              << sync_tabName);

  #endif
    if (sync_tabName.compare("end") == 0)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Information,
                                              tr("Hint"),
                                              tr("Sync Success."
                                              "<p>Please restart the shared filter dialog."));
        msgBox->show();
        QTimer::singleShot(2000, msgBox, SLOT(accept()));

//        GetPersistentPattern().retrieve("sharedFilterSet");
//        sharedFilterSet = PatternPersistentCopy<SharedFilterSet>("sharedFilterSet");
        reject();
    }
}

void SharedFilterDialog::buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
    if ((role == QDialogButtonBox::AcceptRole) ||
        (role == QDialogButtonBox::ApplyRole)) {
      // Copy the sharedfilter set and persist it to disk
      GetPersistentPattern().migrateAndInit("sharedFilterSet");
      *(PatternPersistent<SharedFilterSet>("sharedFilterSet")) = *sharedFilterSet;
      GetPersistentPattern().save("sharedFilterSet");
      emit optionsChanged();
    }

    if (role == QDialogButtonBox::AcceptRole)
      accept();
    else if (role == QDialogButtonBox::RejectRole)
      reject();
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
        mainTabWidget->setCurrentIndex(tabCount);
        tabCount++;
//        localTabSet<<tabText;
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
        delFilterDataByTab(curTab);
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
    QString curTabText = mainTabWidget->tabText(curIndex);
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
//    for (; curFilterCount < 20 ;) {
    if (ok && filterList.size() >= 2) {
        keyEdit = new FilterLineEdit();
        keyEdit->setFixedHeight(25);
        keyEdit->setEnabled(true);
        keyEdit->setText(filterList.at(0));
        filterEdit = new FilterLineEdit();
        filterEdit->setFixedHeight(25);
        filterEdit->setText(filterList.at(1));
        commentEdit = new FilterLineEdit();
        commentEdit->setFixedHeight(25);
        SharedFilter newFilter = SharedFilter(curTabText, filterList.at(0), filterList.at(1), "");
        if (filterList.size() > 2)
        {
            commentEdit->setText(filterList.at(2));
            newFilter.setComment(filterList.at(2));
        }

        newFilter.setFilterItem();
        //save filter data
        sharedFilterSet->sharedFilterList << newFilter;

        QString indexStr = QString::number(curFilterCount);
        filterLineName.clear();
        filterLineName << ("filter_Line_" + indexStr) << ("filter_key_" + indexStr)
                       << ("filter_content_" + indexStr) << ("filter_comment_" + indexStr);
        keyEdit->setObjectName(filterLineName[1]);
        filterEdit->setObjectName(filterLineName[2]);
        commentEdit->setObjectName(filterLineName[3]);

        connect(keyEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));
        connect(filterEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));
        connect(commentEdit, SIGNAL(click(int)), this, SLOT(handleClick(int)));

        filterItemHLayout = new QHBoxLayout();
        filterItemHLayout->setObjectName(filterLineName[0]);
        filterItemHLayout->setSpacing(10);
        filterItemHLayout->addWidget(keyEdit, 1);
        filterItemHLayout->addWidget(filterEdit, 4);
        filterItemHLayout->addWidget(commentEdit, 2);

        mainFilterVLayout->addLayout(filterItemHLayout);

        curFilterCount++;
        filterArray.replace(curIndex, curFilterCount);
        scrollAreaWidgetContents->resize(1160, 50 + 35 * curFilterCount);
    }
//    }
}


QStringList SharedFilterDialog::getFilterLineName(int index)
{
    QString indexStr = QString::number(index);
    filterLineName.clear();
    filterLineName << ("filter_Line_" + indexStr) << ("filter_key_" + indexStr)
                   << ("filter_content_" + indexStr) << ("filter_comment_" + indexStr);
    return filterLineName;
}


void SharedFilterDialog::handleClick(int index)
{
    filterLineNameIndex = index;
}

void SharedFilterDialog::delFilterItem_click()
{
    int curIndex = mainTabWidget->currentIndex();
    curFilterCount = filterArray[curIndex];
    curTabWidget = mainTabWidget->widget(curIndex);
    scrollAreaWidgetContents = curTabWidget->findChild<QWidget *>("scrollwidget");
    mainFilterVLayout = scrollAreaWidgetContents->findChild<QVBoxLayout *>("main_filter_v_layout");
    QStringList strList = getFilterLineName(filterLineNameIndex);
    for (int i = 1; i < 4; i++)
    {
        if (strList.size() > 3)
        {
            FilterLineEdit* lineEdit = scrollAreaWidgetContents->findChild<FilterLineEdit* >(strList[i]);
            if (i == 1) {
                delFilterDataByKey(lineEdit->text());
            }
            lineEdit->deleteLater();
        }
    }
}

void SharedFilterDialog::delFilterDataByKey(QString keyStr)
{
    for (int i = 0; i < sharedFilterSet->sharedFilterList.size(); i++)
    {
        if (sharedFilterSet->sharedFilterList[i].hasMatch(keyStr)) {
            sharedFilterSet->sharedFilterList.removeAt(i);
            i--;
        }
    }
}

void SharedFilterDialog::delFilterDataByTab(QString tabStr)
{
    for (int i = 0; i < sharedFilterSet->sharedFilterList.size(); i++)
    {
        if (!QString::compare(sharedFilterSet->sharedFilterList[i].tab(), tabStr)) {
            sharedFilterSet->sharedFilterList.removeAt(i);
            i--;
        }
    }
}
