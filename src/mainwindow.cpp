/*
 * Copyright (C) 2009, 2010, 2011, 2013, 2014 Nicolas Bonnefon and other
 * contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

// This file implements MainWindow. It is responsible for creating and
// managing the menus, the toolbar, and the CrawlerWidget. It also
// load/save the settings on opening/closing of the app

#include <cassert>
#include <iostream>

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QStyleFactory>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QDebug>

#include "log.h"

#include "mainwindow.h"

#include "DarkStyle.h"
#include "crawlerwidget.h"
#include "externalcom.h"
#include "filtersdialog.h"
#include "menuactiontooltipbehavior.h"
#include "optionsdialog.h"
#include "persistentinfo.h"
#include "recentfiles.h"
#include "sessioninfo.h"
#include "tabbedcrawlerwidget.h"
#include "windowdragger.h"
#include "sharedfilterdialog.h"
#include "persistentpattern.h"
#include "syncfilterdialog.h"

// Returns the size in human readable format
static QString readableSize(qint64 size);

MainWindow::MainWindow(
    std::unique_ptr<Session> session,
    std::shared_ptr<ExternalCommunicator> external_communicator,
    WindowDragger* tb)
    : session_(std::move(session)),
      externalCommunicator_(external_communicator),
      recentFiles_(Persistent<RecentFiles>("recentFiles")),
      mainIcon_(),
      signalMux_(),
      quickFindMux_(session_->getQuickFindPattern()),
      quickMarkMux_(session_->getQuickMarkPattern()),
      mainTabWidget_(tb)
#ifdef GLOGG_SUPPORTS_VERSION_CHECKING
      ,
      versionChecker_()
#endif
{
  titleBar = tb;
  createActions();
  createMenus();
  createIconToolBars();
  createToolBars();

  setAcceptDrops(true);

  // Default geometry
  const QRect geometry = QApplication::desktop()->availableGeometry(this);
  setGeometry(geometry.x() + 20, geometry.y() + 40, geometry.width() - 140,
              geometry.height() - 140);

  mainIcon_.addFile(":/images/hicolor/16x16/glogg.png");
  mainIcon_.addFile(":/images/hicolor/24x24/glogg.png");
  mainIcon_.addFile(":/images/hicolor/32x32/glogg.png");
  mainIcon_.addFile(":/images/hicolor/48x48/glogg.png");

  setWindowIcon(mainIcon_);

  readSettings();

  // Connect the signals to the mux (they will be forwarded to the
  // "current" crawlerwidget

  // Send actions to the crawlerwidget
  signalMux_.connect(this, SIGNAL(followSet(bool)), SIGNAL(followSet(bool)));
  signalMux_.connect(this, SIGNAL(optionsChanged()),
                     SLOT(applyConfiguration()));
  signalMux_.connect(this, SIGNAL(startNewSearch()), SLOT(startNewSearch()));
  signalMux_.connect(this, SIGNAL(enterQuickFind()), SLOT(enteringQuickFind()));
  signalMux_.connect(this, SIGNAL(enterQuickMark()), SLOT(enteringQuickMark()));
  signalMux_.connect(this, SIGNAL(focusFilterBar()), SLOT(focusingFilterBar()));
  signalMux_.connect(this, SIGNAL(focusLogBar()), SLOT(focusingLogBar()));
  signalMux_.connect(this, SIGNAL(hideLogBar()), SLOT(hidingLogBar()));
  signalMux_.connect(this, SIGNAL(focusMainView()), SLOT(focusingMainView()));
  signalMux_.connect(this, SIGNAL(dropSearchBar()), SLOT(dropDownSearchEdit()));
  signalMux_.connect(&quickFindWidget_, SIGNAL(close()),
                     SLOT(exitingQuickFind()));
  signalMux_.connect(&quickMarkWidget_, SIGNAL(close()),
                     SLOT(exitingQuickMark()));

  // Actions from the CrawlerWidget
  signalMux_.connect(SIGNAL(followModeChanged(bool)), this,
                     SLOT(changeFollowMode(bool)));
  signalMux_.connect(SIGNAL(addToQuickSearch(const QString&)), this,
                     SLOT(addToQuickSearch(const QString&)));
  signalMux_.connect(SIGNAL(updateLineNumber(int)), this,
                     SLOT(lineNumberHandler(int)));

  signalMux_.connect(SIGNAL(addToQuickMark(const QString&)), this,
                     SLOT(addToQuickMark(const QString&)));

  signalMux_.connect(SIGNAL(replaceQuickMark(const QString&)), this,
                     SLOT(replaceQuickMark(const QString&)));

  // Register for progress status bar
  signalMux_.connect(SIGNAL(copyToClipboard()), this, SLOT(copy()));
  signalMux_.connect(SIGNAL(fullScreen()), this, SLOT(fullScreen()));
  signalMux_.connect(SIGNAL(copyWithColor()), this, SLOT(copyWithColor()));
  signalMux_.connect(SIGNAL(openFile()), this, SLOT(open()));
  signalMux_.connect(SIGNAL(exitApp()), titleBar->parent()->parent(),
                     SLOT(close()));
  signalMux_.connect(SIGNAL(changeFollowMode()), this,
                     SLOT(changeFollowMode()));
  signalMux_.connect(SIGNAL(disableFollowMode()), this,
                     SLOT(disableFollowMode()));
  signalMux_.connect(SIGNAL(loadingProgressed(int)), this,
                     SLOT(updateLoadingProgress(int)));
  signalMux_.connect(SIGNAL(loadingFinished(LoadingStatus)), this,
                     SLOT(handleLoadingFinished(LoadingStatus)));

  // Register for checkbox changes
  signalMux_.connect(SIGNAL(searchRefreshChanged(int)), this,
                     SLOT(handleSearchRefreshChanged(int)));
  signalMux_.connect(SIGNAL(ignoreCaseChanged(int)), this,
                     SLOT(handleIgnoreCaseChanged(int)));

  // Configure the main tabbed widget
  QHBoxLayout* layout = new QHBoxLayout;
  mainTabWidget_.setLayout(layout);
  mainTabWidget_.setDocumentMode(true);
  mainTabWidget_.setMovable(true);
  mainTabWidget_.setTabsClosable(true);

  connect(&mainTabWidget_, SIGNAL(tabCloseRequested(int)), this,
          SLOT(closeTab(int)));
  connect(&mainTabWidget_, SIGNAL(currentChanged(int)), this,
          SLOT(currentTabChanged(int)));

  // Establish the QuickFindWidget and mux ( to send requests from the
  // QFWidget to the right window )
  connect(&quickFindWidget_,
          SIGNAL(patternConfirmed(const QString&, bool, QFDirection)),
          &quickFindMux_,
          SLOT(confirmPattern(const QString&, bool, QFDirection)));
  connect(&quickFindWidget_, SIGNAL(patternUpdated(const QString&, bool)),
          &quickFindMux_, SLOT(setNewPattern(const QString&, bool)));
  connect(&quickFindWidget_, SIGNAL(cancelSearch()), &quickFindMux_,
          SLOT(cancelSearch()));
  connect(&quickFindWidget_, SIGNAL(searchForward()), &quickFindMux_,
          SLOT(searchForward()));
  connect(&quickFindWidget_, SIGNAL(searchBackward()), &quickFindMux_,
          SLOT(searchBackward()));
  connect(&quickFindWidget_, SIGNAL(searchNext()), &quickFindMux_,
          SLOT(searchNext()));

  connect(&quickMarkWidget_, SIGNAL(patternChanged(const QString&, bool)),
          &quickMarkMux_,
          SLOT(confirmPatternWithoutSearch(const QString&, bool)));
  connect(&quickMarkWidget_,
          SIGNAL(patternConfirmed(const QString&, bool, QFDirection)),
          &quickMarkMux_,
          SLOT(confirmPattern(const QString&, bool, QFDirection)));
  connect(&quickMarkWidget_, SIGNAL(patternUpdated(const QString&, bool)),
          &quickMarkMux_, SLOT(setNewPattern(const QString&, bool)));
  connect(&quickMarkWidget_, SIGNAL(cancelSearch()), &quickMarkMux_,
          SLOT(cancelMark()));
  connect(&quickMarkWidget_, SIGNAL(searchForward()), &quickMarkMux_,
          SLOT(markForward()));
  connect(&quickMarkWidget_, SIGNAL(searchBackward()), &quickMarkMux_,
          SLOT(markBackward()));
  connect(&quickMarkWidget_, SIGNAL(searchNext()), &quickMarkMux_,
          SLOT(markNext()));

  // QuickFind changes coming from the views
  connect(&quickFindMux_, SIGNAL(patternChanged(const QString&)), this,
          SLOT(changeQFPattern(const QString&)));
  connect(&quickFindMux_, SIGNAL(notify(const QFNotification&)),
          &quickFindWidget_, SLOT(notify(const QFNotification&)));
  connect(&quickFindMux_, SIGNAL(clearNotification()), &quickFindWidget_,
          SLOT(clearNotification()));

  connect(&quickMarkMux_, SIGNAL(patternChanged(const QString&)), this,
          SLOT(changeQMPattern(const QString&)));
  connect(&quickMarkMux_, SIGNAL(notify(const QFNotification&)),
          &quickMarkWidget_, SLOT(notify(const QFNotification&)));
  connect(&quickMarkMux_, SIGNAL(clearNotification()), &quickMarkWidget_,
          SLOT(clearNotification()));

  // Actions from external instances
  connect(externalCommunicator_.get(), SIGNAL(loadFile(const QString&)), this,
          SLOT(loadFileNonInteractive(const QString&)));
  connect(qApp, SIGNAL(loadFile(const QString&)), this,
          SLOT(loadFileNonInteractive(const QString&)));

#ifdef GLOGG_SUPPORTS_VERSION_CHECKING
  // Version checker notification
  connect(&versionChecker_, SIGNAL(newVersionFound(const QString&)), this,
          SLOT(newVersionNotification(const QString&)));
#endif

  // Construct the QuickFind bar
  quickFindWidget_.setTitle("Find");
  quickFindWidget_.hide();
  quickFindWidget_.setchecked();
  quickMarkWidget_.setTitle("Mark");
  quickMarkWidget_.hide();
  quickMarkWidget_.setchecked();

  QWidget* central_widget = new QWidget();
  QVBoxLayout* main_layout = new QVBoxLayout();
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->addWidget(&mainTabWidget_);
  main_layout->addWidget(&quickFindWidget_);
  main_layout->addWidget(&quickMarkWidget_);
  main_layout->addWidget(infoLine);
  //  main_layout->addWidget(lineNbField);
  infoLine->setVisible(false);
  central_widget->setLayout(main_layout);

  setCentralWidget(central_widget);
}

void MainWindow::reloadGeometry() {
  QByteArray geometry;

  session_->storedGeometry(&geometry);
  restoreGeometry(geometry);
}

void MainWindow::reloadSession() {
  int current_file_index = -1;

  for (auto open_file : session_->restore([]() { return new CrawlerWidget(); },
                                          &current_file_index)) {
    QString file_name = {open_file.first.c_str()};
    CrawlerWidget* crawler_widget =
        dynamic_cast<CrawlerWidget*>(open_file.second);

    assert(crawler_widget);

    mainTabWidget_.addTab(crawler_widget, strippedName(file_name), file_name);
  }

  if (current_file_index >= 0)
    mainTabWidget_.setCurrentIndex(current_file_index);
}

void MainWindow::loadInitialFile(QString fileName) {
  LOG(logDEBUG) << "loadInitialFile";

  // Is there a file passed as argument?
  if (!fileName.isEmpty()) loadFile(fileName);
}

void MainWindow::startBackgroundTasks() {
  LOG(logDEBUG) << "startBackgroundTasks";

#ifdef GLOGG_SUPPORTS_VERSION_CHECKING
  versionChecker_.startCheck();
#endif
}

QIcon MainWindow::getIcon() { return mainIcon_; }

void MainWindow::setApplication(GloggApp* app) { app_ = app; }

//
// Private functions
//

const MainWindow::EncodingList MainWindow::encoding_list[] = {
    {"&Auto"},
    {"ASCII / &ISO-8859-1"},
    {"&UTF-8"},
    {"UTF-16LE"},
    {"UTF-16BE"},
    {"CP1251"},
    {"CP1252"},
    {"&Big5"},
    {"&GB18030 / GB2312"},
    {"&Shift_JIS"},
    {"&KOI8-R"}};

// Menu actions
void MainWindow::createActions() {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");

  openAction = new QAction(tr("&Open..."), this);
  openAction->setShortcut(QKeySequence::Open);
  openAction->setIcon(QIcon(":/images/open16.png"));
  openAction->setStatusTip(tr("Open a file"));
  connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

  copyPathAction = new QAction(tr("Copy path"), this);
  connect(copyPathAction, SIGNAL(triggered()), this, SLOT(copyPath()));

  saveAsAction = new QAction(tr("&Save As"), this);
  saveAsAction->setShortcut(tr("Ctrl+S"));
  saveAsAction->setStatusTip(tr("save as and open file"));
  saveAsAction->setIcon(QIcon(":/images/save.png"));
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAsFile()));

  saveSelectedAsAction = new QAction(tr("Save Selection As"), this);
  saveSelectedAsAction->setShortcut(tr("Ctrl+Shift+S"));
  saveSelectedAsAction->setStatusTip(
      tr("save selected content as new file and open the file"));
  connect(saveSelectedAsAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedAsFile()));

  calculateTimeDiffAction =
      new QAction(tr("Calculation time consumption"), this);
  connect(calculateTimeDiffAction, SIGNAL(triggered()), this,
          SLOT(calculateTimeDiff()));

  saveFilteredAsAction = new QAction(tr("Open Filtered In New Tab"), this);
  saveFilteredAsAction->setShortcut(tr("Ctrl+Shift+T"));
  saveFilteredAsAction->setStatusTip(tr("save Filtered as and open file"));
  connect(saveFilteredAsAction, SIGNAL(triggered()), this,
          SLOT(saveFilteredAsFile()));

  retraceLogAction = new QAction(tr("Retrace Selection"), this);
  retraceLogAction->setStatusTip(tr("retrace log"));
  retraceLogAction->setShortcut(tr("Ctrl+R"));
  connect(retraceLogAction, SIGNAL(triggered()), this, SLOT(retraceLog()));

  reformatLogAction = new QAction(tr("Reformat"), this);
  reformatLogAction->setShortcut(tr("Ctrl+Shift+R"));
  reformatLogAction->setStatusTip(tr("reformat log"));
  connect(reformatLogAction, SIGNAL(triggered()), this, SLOT(reformatLog()));

  viewPicturesAction = new QAction(tr("Open Pictures"), this);
  viewPicturesAction->setShortcut(tr("Ctrl+P"));
  viewPicturesAction->setStatusTip(tr("reformat log"));
  connect(viewPicturesAction, SIGNAL(triggered()), this, SLOT(viewPictures()));

  cutLogAction = new QAction(tr("Cut Upper Lines"), this);
  cutLogAction->setShortcut(tr("Ctrl+Shift+X"));
  cutLogAction->setStatusTip(tr("cut upper lines"));
  connect(cutLogAction, SIGNAL(triggered()), this, SLOT(cutLog()));

  closeAction = new QAction(tr("&Close"), this);
  closeAction->setShortcut(tr("Ctrl+W"));
  closeAction->setStatusTip(tr("Close document"));
  connect(closeAction, SIGNAL(triggered()), this, SLOT(closeTab()));

  closeLeftAction = new QAction(tr("Close tabs to the left"), this);
  closeLeftAction->setStatusTip(tr("closeLeft document"));
  connect(closeLeftAction, SIGNAL(triggered()), this, SLOT(closeTabToLeft()));

  closeRightAction = new QAction(tr("Close tabs to the right"), this);
  closeRightAction->setStatusTip(tr("closeRight document"));
  connect(closeRightAction, SIGNAL(triggered()), this, SLOT(closeTabToRight()));

  closeAllAction = new QAction(tr("Close &All"), this);
  closeAllAction->setStatusTip(tr("Close all documents"));
  connect(closeAllAction, SIGNAL(triggered()), this, SLOT(closeAll()));

  // Recent files
  for (int i = 0; i < MaxRecentFiles; ++i) {
    recentFileActions[i] = new QAction(this);
    recentFileActions[i]->setVisible(false);
    connect(recentFileActions[i], SIGNAL(triggered()), this,
            SLOT(openRecentFile()));
  }

  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  exitAction->setStatusTip(tr("Exit the application"));
  connect(exitAction, SIGNAL(triggered()), titleBar->parent()->parent(),
          SLOT(close()));

  copyAction = new QAction(tr("&Copy"), this);
  copyAction->setShortcut(QKeySequence::Copy);
  copyAction->setStatusTip(tr("Copy the selection"));
  connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

  copyWithColorAction = new QAction(tr("Copy To Jira"), this);
  copyWithColorAction->setShortcut(
      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
  copyWithColorAction->setStatusTip(
      tr("Copy the selection with foreground color"));
  connect(copyWithColorAction, SIGNAL(triggered()), this,
          SLOT(copyWithColor()));

  selectAllAction = new QAction(tr("Select &All"), this);
  selectAllAction->setShortcut(tr("Ctrl+A"));
  selectAllAction->setStatusTip(tr("Select all the text"));
  connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));

  startLogcatAction = new QAction(tr("Start Logcat"), this);
  startLogcatAction->setShortcut(QKeySequence(Qt::Key_F1));
  startLogcatAction->setStatusTip(tr("startLogcat the selection"));
  //enabled_startLogcat = new QPixmap(":/images/start16.png");
  //disabled_startLogcat = new QPixmap(":/images/disabled_start16.png");
  //startLogcatIcon = new QIcon(*enabled_startLogcat);
  //startLogcatIcon->addPixmap(*enabled_startLogcat, QIcon::Disabled, QIcon::Off);
  startLogcatAction->setIcon(QIcon(":/images/start16.png"));

  connect(startLogcatAction, SIGNAL(triggered()), this, SLOT(startLogcat()));

  stopLogcatAction = new QAction(tr("Stop Logcat"), this);
  stopLogcatAction->setShortcut(QKeySequence(Qt::Key_F2));
  stopLogcatAction->setStatusTip(tr("stopLogcat the selection"));
  connect(stopLogcatAction, SIGNAL(triggered()), this, SLOT(stopLogcat()));
  stopLogcatAction->setIcon(QIcon(":/images/stop16.png"));

  findAction = new QAction(tr("&Find..."), this);
  findAction->setShortcut(QKeySequence::Find);
  findAction->setStatusTip(tr("Find the text"));
  connect(findAction, SIGNAL(triggered()), this, SLOT(find()));

  markAction = new QAction(tr("&Mark..."), this);
  markAction->setShortcut(tr("Ctrl+M"));
  markAction->setStatusTip(tr("Mark the text"));
  connect(markAction, SIGNAL(triggered()), this, SLOT(mark()));

  searchShareAction = new QAction(tr("Search and share..."), this);
  searchShareAction->setShortcut(QKeySequence(Qt::Key_Question));
  searchShareAction->setStatusTip(tr("search and share in the git repo"));
  connect(searchShareAction, SIGNAL(triggered()), this, SIGNAL(focusLogBar()));

  overviewVisibleAction = new QAction(tr("Matches &overview"), this);
  overviewVisibleAction->setCheckable(true);
  overviewVisibleAction->setChecked(config->isOverviewVisible());
  connect(overviewVisibleAction, SIGNAL(toggled(bool)), this,
          SLOT(toggleOverviewVisibility(bool)));

  lineNumbersVisibleInMainAction =
      new QAction(tr("Line &numbers in main view"), this);
  lineNumbersVisibleInMainAction->setCheckable(true);
  lineNumbersVisibleInMainAction->setChecked(config->mainLineNumbersVisible());
  connect(lineNumbersVisibleInMainAction, SIGNAL(toggled(bool)), this,
          SLOT(toggleMainLineNumbersVisibility(bool)));

  lineNumbersVisibleInFilteredAction =
      new QAction(tr("Line &numbers in filtered view"), this);
  lineNumbersVisibleInFilteredAction->setCheckable(true);
  lineNumbersVisibleInFilteredAction->setChecked(
      config->filteredLineNumbersVisible());
  connect(lineNumbersVisibleInFilteredAction, SIGNAL(toggled(bool)), this,
          SLOT(toggleFilteredLineNumbersVisibility(bool)));

  followAction = new QAction(tr("&Follow File"), this);
  followAction->setShortcut(Qt::Key_F);
  followAction->setCheckable(true);
  followAction->setIcon(QIcon(":/images/scroll_to_end16.png"));
  followAction->setStatusTip(tr("scroll to the end"));
  connect(followAction, SIGNAL(toggled(bool)), this, SIGNAL(followSet(bool)));

  reloadAction = new QAction(tr("&Reload"), this);
  reloadAction->setShortcut(QKeySequence::Refresh);
  reloadAction->setIcon(QIcon(":/images/reload14.png"));
  signalMux_.connect(reloadAction, SIGNAL(triggered()), SLOT(reload()));

  fullScreenAction = new QAction(tr("Full Screen"), this);
  fullScreenAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F12));
  connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(fullScreen()));

  stopAction = new QAction(tr("&Stop"), this);
  stopAction->setIcon(QIcon(":/images/stop14.png"));
  stopAction->setEnabled(true);
  signalMux_.connect(stopAction, SIGNAL(triggered()), SLOT(stopLoading()));

  filtersAction = new QAction(tr("&Local filter"), this);
  filtersAction->setStatusTip(tr("Show the local filters"));
  filtersAction->setShortcut(QKeySequence(Qt::Key_F8));
  connect(filtersAction, SIGNAL(triggered()), this, SLOT(filters()));

  sharedFilterAction = new QAction(tr("&Shared filter"), this);
  sharedFilterAction->setStatusTip(tr("Show the shared filters"));
  sharedFilterAction->setShortcut(QKeySequence(Qt::Key_F9));
  connect(sharedFilterAction, SIGNAL(triggered()), this, SLOT(openSharedFilter()));

  optionsAction = new QAction(tr("&Options..."), this);
  optionsAction->setStatusTip(tr("Show the Options box"));
  connect(optionsAction, SIGNAL(triggered()), this, SLOT(options()));

  shortcutAction = new QAction(tr("&Shortcuts"), this);
  shortcutAction->setStatusTip("Open the shortcut key document");
  connect(shortcutAction, SIGNAL(triggered()), this, SLOT(showShortcuts()));

  aboutAction = new QAction(tr("&About"), this);
  aboutAction->setStatusTip(tr("Show the About box"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAction = new QAction(tr("About &Qt"), this);
  aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));

  aboutCustomizedAction = new QAction(tr("More info"));
  aboutCustomizedAction->setStatusTip(tr("Show more info about customized glogg"));
  connect(aboutCustomizedAction, SIGNAL(triggered()), this, SLOT(aboutCustomizedGlogg()));

  encodingGroup = new QActionGroup(this);

  for (int i = 0; i < static_cast<int>(Encoding::ENCODING_MAX); ++i) {
    encodingAction[i] = new QAction(tr(encoding_list[i].name), this);
    encodingAction[i]->setCheckable(true);
    encodingGroup->addAction(encodingAction[i]);
  }

  encodingAction[0]->setStatusTip(
      tr("Automatically detect the file's encoding"));
  encodingAction[0]->setChecked(true);

  connect(encodingGroup, SIGNAL(triggered(QAction*)), this,
          SLOT(encodingChanged(QAction*)));

  dumpCameraAction = new QAction(tr("Parameters"), this);
  dumpCameraAction->setStatusTip(tr("dumpsys media.camera"));
  connect(dumpCameraAction, SIGNAL(triggered()), this, SLOT(dumpCamera()));

  dumpStreamAction = new QAction(tr("Stream"), this);
  connect(dumpStreamAction, SIGNAL(triggered()), this, SLOT(dumpStream()));

  dumpDeviceInfoAction = new QAction(tr("Device info"));
  connect(dumpDeviceInfoAction, SIGNAL(triggered()), this, SLOT(dumpDeviceInfo()));
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAction);
  fileMenu->addAction(viewPicturesAction);
  fileMenu->addAction(copyPathAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addAction(saveSelectedAsAction);
  fileMenu->addAction(calculateTimeDiffAction);
  fileMenu->addSeparator();
  fileMenu->addAction(saveFilteredAsAction);
  fileMenu->addAction(retraceLogAction);
  fileMenu->addAction(reformatLogAction);
  fileMenu->addAction(cutLogAction);
  fileMenu->addSeparator();
  fileMenu->addAction(closeAction);
  fileMenu->addAction(closeLeftAction);
  fileMenu->addAction(closeRightAction);
  fileMenu->addAction(closeAllAction);
  fileMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i) {
    fileMenu->addAction(recentFileActions[i]);
    recentFileActionBehaviors[i] =
        new MenuActionToolTipBehavior(recentFileActions[i], fileMenu, this);
  }
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(copyAction);
  editMenu->addAction(copyWithColorAction);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAction);
  editMenu->addSeparator();
  editMenu->addAction(startLogcatAction);
  editMenu->addAction(stopLogcatAction);
  editMenu->addSeparator();
  editMenu->addAction(findAction);
  editMenu->addAction(markAction);
  editMenu->addAction(searchShareAction);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(fullScreenAction);
  viewMenu->addAction(overviewVisibleAction);
  viewMenu->addSeparator();
  viewMenu->addAction(lineNumbersVisibleInMainAction);
  viewMenu->addAction(lineNumbersVisibleInFilteredAction);
  viewMenu->addSeparator();
  viewMenu->addAction(followAction);
  viewMenu->addSeparator();
  viewMenu->addAction(reloadAction);

  toolsMenu = menuBar()->addMenu(tr("Tools"));
  fileMenu = toolsMenu->addMenu(tr("Filter"));
  fileMenu->addAction(filtersAction);
  fileMenu->addAction(sharedFilterAction);
  toolsMenu->addSeparator();
  toolsMenu->addAction(optionsAction);
  toolsMenu->addSeparator();

  encodingMenu = menuBar()->addMenu(tr("En&coding"));
  encodingMenu->addAction(encodingAction[0]);
  encodingMenu->addSeparator();
  for (int i = 1; i < static_cast<int>(Encoding::ENCODING_MAX); ++i) {
    encodingMenu->addAction(encodingAction[i]);
  }

  menuBar()->addSeparator();

  cameraMenu = menuBar()->addMenu(tr("&Camera"));
  cameraMenu->addAction(dumpCameraAction);
  cameraMenu->addAction(dumpStreamAction);
  cameraMenu->addSeparator();
  cameraMenu->addAction(dumpDeviceInfoAction);

  helpMenu = menuBar()->addMenu(tr("&Help"));
//  helpMenu->addAction(shortcutAction);
//  helpMenu->addSeparator();
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutCustomizedAction);

}

void MainWindow::createIconToolBars() {
    menuToolBar = addToolBar(tr("Menu ToolBar"));
    menuToolBar->addAction(openAction);
    menuToolBar->addAction(saveAsAction);
    menuToolBar->addAction(startLogcatAction);
    menuToolBar->addAction(stopLogcatAction);
    menuToolBar->addAction(followAction);
}

void MainWindow::createToolBars() {
  infoLine = new QLineEdit();
  //  infoLine->setLineWidth(0);

  lineNbField = new QLabel();
  lineNbField->setText("Line 0");
  lineNbField->setStyleSheet("QLabel { color : rgb(0, 119, 201) ; }");
  lineNbField->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lineNbField->setMinimumSize(
      lineNbField->fontMetrics().size(0, "Line 0000000"));
}

//
// Slots
//

// Opens the file selection dialog to select a new log file
void MainWindow::open() {
  QString defaultDir = ".";

  // Default to the path of the current file if there is one
  if (auto current = currentCrawlerWidget()) {
    std::string current_file = session_->getFilename(current);
    QFileInfo fileInfo = QFileInfo(QString(current_file.c_str()));
    defaultDir = fileInfo.path();
  }

  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open file"), defaultDir, tr("All files (*)"));
  if (!fileName.isEmpty()) {
    if (fileName.endsWith(".jpeg") || fileName.endsWith(".mp4") ||
        fileName.endsWith(".png") || fileName.endsWith(".jpg") ||
        fileName.endsWith(".html")) {
      QProcess process;
      QString path =
          QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

      std::shared_ptr<Configuration> config =
          Persistent<Configuration>("settings");
      QString unzipPath = config->unzipPath();
#ifdef _WIN32
      QDir dir = QDir(QCoreApplication::applicationDirPath());
      LOG(logERROR) << "path: " << QDir::currentPath().toStdString();
      process.setWorkingDirectory(path);
      QString command = path + "open-file.bat ";
      unzipPath =
          unzipPath + QDir::separator() + QFileInfo(fileName).baseName();
      process.startDetached(command, QStringList()
                                         << dir.toNativeSeparators(fileName));
#else
      process.startDetached("/bin/bash",
                            QStringList() << path + "open-file.sh" << fileName);
#endif
    } else {
      loadFile(fileName);
    }
  }
}

// Opens a log file from the recent files list
void MainWindow::openRecentFile() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (action) loadFile(action->data().toString());
}

void MainWindow::closeTabToRight() {
  int currentIndex = mainTabWidget_.currentIndex();
  while (mainTabWidget_.count() - 1 > currentIndex) {
    currentIndex = mainTabWidget_.currentIndex() + 1;
    if (currentIndex >= 0) {
      closeTab(currentIndex);
    }
  }
}

void MainWindow::closeTabToLeft() {
  int currentIndex = mainTabWidget_.count() - mainTabWidget_.currentIndex();
  while (mainTabWidget_.count() > currentIndex) {
    closeTab(0);
  }
}

void MainWindow::closeOtherTabs() {
  closeTabToRight();
  closeTabToLeft();
}

// Close current tab
void MainWindow::closeTab() {
  int currentIndex = mainTabWidget_.currentIndex();

  if (currentIndex >= 0) {
    closeTab(currentIndex);
  }
}

// Close all tabs
void MainWindow::closeAll() {
  while (mainTabWidget_.count()) {
    closeTab(0);
  }
}

// Select all the text in the currently selected view
void MainWindow::selectAll() {
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) current->selectAll();
}

// Copy the currently selected line into the clipboard
void MainWindow::copy() {
  static QClipboard* clipboard = QApplication::clipboard();
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    const QString string = current->getSelectedText();
    clipboard->setText(string);

    // Put it in the global selection as well (X11 only)
    clipboard->setText(string, QClipboard::Selection);
  }
}

void MainWindow::copyPath() {
  static QClipboard* clipboard = QApplication::clipboard();
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    clipboard->setText(current_file);

    // Put it in the global selection as well (X11 only)
    clipboard->setText(current_file, QClipboard::Selection);
  }
}

void MainWindow::copyWithColor() {
  static QClipboard* clipboard = QApplication::clipboard();
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    QString colorString;
    colorString.append("{panel:title=");

    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    colorString.append(strippedName(current_file));
    colorString.append("|titleBGColor=#7ccee9|bgColor=#c7ffce}\n");
    const QString string = current->getSelectedTextWithColor();
    colorString.append(string);
    colorString.append("\n{panel}");
    clipboard->setText(colorString);
    // Put it in the global selection as well (X11 only)
    clipboard->setText(colorString, QClipboard::Selection);
  }
}

void MainWindow::calculateTimeDiff() {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current) {
    QString selectedString = current->getSelectedText();
    bool ok;
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    QDateTime dateTime = dateTime.currentDateTime();
    QString dstPath = currentPath("time_cost");
    QFile file(dstPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << selectedString;
    file.close();

    process_ = new QProcess();
    QObject::connect(process_, SIGNAL(readyReadStandardOutput()), this,
                     SLOT(updateInfoLine1()));

    QObject::connect(
        process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
          infoLine->setVisible(false);
          process_->deleteLater();
        });
    infoLine->setVisible(true);
    process_->start("/bin/bash", QStringList()
                                     << path + "calculate_time_diff.sh"
                                     << dstPath);
  }
}

void MainWindow::saveSelectedAsFile() {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current) {
    QString selectedString = current->getSelectedText();
    bool ok;
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    QDateTime dateTime = dateTime.currentDateTime();
    QString text = QInputDialog::getText(
        this, tr("Save As"),
        tr("                                               "
           "                            "),
        QLineEdit::Normal, dateTime.toString("yyyy-MM-dd_HH_mm_ss_"), &ok);
    if (ok) {
      QString dstPath = currentPath(text);
      QFile file(dstPath);
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);
      out << selectedString;
      file.close();
      loadFile(dstPath);
    }
  }
}

void MainWindow::retraceLog() {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current) {
    QString selectedString = current->getSelectedText();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    QDateTime dateTime = dateTime.currentDateTime();
    QString text = "retrace_" + dateTime.toString("yyyy-MM-dd_HH_mm_ss");
    QString dstPath = currentPath(text);
    QFile file(dstPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << selectedString;
    file.close();
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    QString currentPath = QFileInfo(current_file).absoluteDir().path();
    QProcess process;
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");

    QString unzipPath = config->unzipPath();
    process.startDetached(
        "/bin/bash", QStringList() << path + "retrace.sh" << dstPath
                                   << currentPath << current_file << unzipPath);
  }
}

void MainWindow::reformatLog() {
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    QProcess process;
    process.startDetached(
        "/bin/bash", QStringList() << path + "reformat_log.sh" << current_file);
  }
}

void MainWindow::viewPictures() {
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    QString unzipPath = config->unzipPath();
    QProcess process;
    process.startDetached("/bin/bash", QStringList()
                                           << path + "view_pictures_log.sh"
                                           << current_file << unzipPath);
  }
}

void MainWindow::cutLog() {
  CrawlerWidget* current = currentCrawlerWidget();

  if (current) {
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    QProcess process;
    process.startDetached("/bin/bash", QStringList()
                                           << path + "cut-upper-lines.sh"
                                           << QString::number(lineNumber_)
                                           << current_file);
  }
}

void MainWindow::saveFilteredAsFile() {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current) {
    QString selectedString = current->getFilteredText();
    if (!selectedString.isEmpty()) {
      QString path =
          QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
      QDateTime dateTime = dateTime.currentDateTime();
      QString text = "result_" + dateTime.toString("yyyy-MM-dd_HH_mm_ss");

      QString dstPath = currentPath(text);
      QFile file(dstPath);
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);
      out << selectedString;
      file.close();
      loadFile(dstPath);
    }
  }
}

void MainWindow::saveAsFile() {
  bool ok;
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
  QDateTime dateTime = dateTime.currentDateTime();
  QString text = QInputDialog::getText(
      this, tr("Save As"),
      tr("                                               "
         "                            "),
      QLineEdit::Normal, dateTime.toString("yyyy-MM-dd_HH_mm_ss_"), &ok);
  if (ok) {
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    QString current_file =
        session_->getFilename(currentCrawlerWidget()).c_str();
    QString dstPath = currentPath(text);
    QProcess process;
    process.startDetached("/bin/bash", QStringList()
                                           << path + "save-and-reopen.sh"
                                           << current_file << dstPath);
  }
}

void MainWindow::saveAs(const QString& fileName) {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    QString string =
        current->getSelectedText().remove(QRegExp("^[0-9: . -]+[^A-Z]"));
    QString text = string.replace(QRegExp("\n[0-9: . -]+[^A-Z]"), "\n");
    out << text;
    file.close();
  }
}

// Display the QuickFind bar
void MainWindow::find() {
  CrawlerWidget* current = currentCrawlerWidget();
  if (current && current->isSelectedPortion()) {
    const QString string = current->getSelectedText();
    QString findString = QRegularExpression::escape(string);
    if (!quickFindWidget_.getSearchingText().contains(findString)) {
      emit addToQuickSearch(string);
      return;
    }
  }
  displayQuickFindBar(QFDirection::Forward);
}

void MainWindow::mark() { displayQuickMarkBar(QFDirection::Forward); }

// Opens the 'Filters' dialog box
void MainWindow::filters() {
  FiltersDialog dialog(this);
  signalMux_.connect(&dialog, SIGNAL(optionsChanged()),
                     SLOT(applyConfiguration()));
  dialog.exec();
  signalMux_.disconnect(&dialog, SIGNAL(optionsChanged()),
                        SLOT(applyConfiguration()));
}

void MainWindow::openSharedFilter() {

    SharedFilterDialog dialog(this);
    signalMux_.connect(&dialog, SIGNAL(optionsChanged()),
                       SLOT(applyConfiguration()));
    dialog.exec();
    signalMux_.disconnect(&dialog, SIGNAL(optionsChanged()),
                          SLOT(applyConfiguration()));
}

// Opens the 'Options' modal dialog box
void MainWindow::options() {
  OptionsDialog dialog(this);
//  signalMux_.connect(&dialog, SIGNAL(optionsChanged()),
//                     SLOT(applyConfiguration()));
//  connect(&dialog, SIGNAL(optionsChanged()), SLOT(applyConfiguration()));
  dialog.exec();
//  signalMux_.disconnect(&dialog, SIGNAL(optionsChanged()),
//                        SLOT(applyConfiguration()));
//  disconnect(&dialog, SIGNAL(optionsChanged()), this,
//             SLOT(applyConfiguration()));
}

void MainWindow::showShortcuts() {
    QProcess* process = new QProcess();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    process->startDetached("/bin/bash", QStringList() << path + "shortcut.sh");
}

// Opens the 'About' dialog box.
void MainWindow::about() {
  QMessageBox::about(
      this, tr("About glogg"),
      tr("<h2>glogg " GLOGG_VERSION "</h2>"
         "<p>A fast, advanced log explorer."
#ifdef GLOGG_COMMIT
         "<p>Built " GLOGG_DATE " from " GLOGG_COMMIT
#endif
         "<p><a "
         "href=\"http://glogg.bonnefon.org/\">http://glogg.bonnefon.org/</a></"
         "p>"
         "<p>Copyright &copy; 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 "
         "Nicolas Bonnefon and other contributors"
         "<p>You may modify and redistribute the program under the terms of "
         "the GPL (version 3 or later)."
         "<p>Support incremental search for QuickFind (‘/’ or ‘Ctrl+F’)."
         "<p>Support multiple Mark for QuickMark (‘Ctrl+M’)."
         "<p>Add (optional) line numbers to either or both views."
         "<p>Show the location of matches on the overview when hovering onto "
         "them."
         "<p>Add a ‘select all’ feature and block selecting with shift+click."
         "<p>Fix QuickFind directions to behave like vim (‘*’, ‘#’, ‘n’ and "
         "‘N’)."
         "<p>Support moving like vim(‘j’, ‘k’, ‘h’, ‘l’, ‘Ctrl+D’, and "
         "‘Ctrl+U’)."
         "<p>Maximize filterWindow (‘Ctrl+Z’)."
         "<p>Switch window (‘Space’)."
         "<p>Quick Filter(‘.’, ‘Ctrl+.’ and ‘Ctrl+j’)."));
}

// Opens the 'About Qt' dialog box.
void MainWindow::aboutQt() {}


void MainWindow::aboutCustomizedGlogg() {
    QMessageBox::about(
        this, tr("About customized glogg"),
        tr("<h2>customized glogg " GLOGG_VERSION "</h2>"
           "<p>See more information about customized glogg."
           "<p><a "
           "href=\"https://xiaomi.f.mioffice.cn/docs/dock4XNd2Ap5QXr5vWbdmWw1AEg/\">https://xiaomi.f.mioffice.cn/docs/</a></"));
}

void MainWindow::encodingChanged(QAction* action) {
  int i = 0;
  for (i = 0; i < static_cast<int>(Encoding::ENCODING_MAX); ++i)
    if (action == encodingAction[i]) break;

  LOG(logDEBUG) << "encodingChanged, encoding " << i;
  currentCrawlerWidget()->setEncoding(static_cast<Encoding>(i));
  // updateInfoLine();
}
void MainWindow::applyConfiguration() {
  LOG(logERROR) << "applyConfiguration";
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  transparent_ = config->transparent();
  app_->setStyle(new DarkStyle(transparent_));
}

void MainWindow::toggleOverviewVisibility(bool isVisible) {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  config->setOverviewVisible(isVisible);
  emit optionsChanged();
}

void MainWindow::toggleMainLineNumbersVisibility(bool isVisible) {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  config->setMainLineNumbersVisible(isVisible);
  emit optionsChanged();
}

void MainWindow::toggleFilteredLineNumbersVisibility(bool isVisible) {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  config->setFilteredLineNumbersVisible(isVisible);
  emit optionsChanged();
}

void MainWindow::changeFollowMode(bool follow) {
  followAction->setChecked(follow);
}

void MainWindow::lineNumberHandler(int line) {
  // The line number received is the internal (starts at 0)
  lineNumber_ = line;
  lineNbField->setText(tr("Line %1").arg(line + 1));
}

void MainWindow::updateLoadingProgress(int progress) {
  LOG(logDEBUG) << "Loading progress: " << progress;

  QString current_file = session_->getFilename(currentCrawlerWidget()).c_str();

  // We ignore 0% and 100% to avoid a flash when the file (or update)
  // is very short.
  if (progress > 0 && progress < 100) {
    //    infoLine->setText(current_file +
    //                      tr(" - Indexing lines... (%1 %)").arg(progress));
    //    infoLine->displayGauge(progress);

    stopAction->setEnabled(true);
    reloadAction->setEnabled(false);
  }
}

void MainWindow::handleLoadingFinished(LoadingStatus status) {
  LOG(logDEBUG) << "handleLoadingFinished success="
                << (status == LoadingStatus::Successful);

  // No file is loading
  loadingFileName.clear();

  if (status == LoadingStatus::Successful) {
    //    infoLine->hideGauge();
    stopAction->setEnabled(false);
    reloadAction->setEnabled(true);

    // Now everything is ready, we can finally show the file!
    currentCrawlerWidget()->show();
  } else {
    if (status == LoadingStatus::NoMemory) {
      QMessageBox alertBox;
      alertBox.setText("Not enough memory.");
      alertBox.setInformativeText(
          "The system does not have enough \
memory to hold the index for this file. The file will now be closed.");
      alertBox.setIcon(QMessageBox::Critical);
      alertBox.exec();
    }

    closeTab(mainTabWidget_.currentIndex());
  }
}

void MainWindow::handleSearchRefreshChanged(int state) {
  auto config = Persistent<Configuration>("settings");
  config->setSearchAutoRefreshDefault(state == Qt::Checked);
}

void MainWindow::handleIgnoreCaseChanged(int state) {
  auto config = Persistent<Configuration>("settings");
  config->setSearchIgnoreCaseDefault(state == Qt::Checked);
}

void MainWindow::addToQuickSearch(const QString& string) {
  displayQuickFindBar(QFDirection::Forward);
  quickFindWidget_.addToQuickSearch(string);
}

void MainWindow::updateInfoLine1() {
  QByteArray data = process_->readAllStandardOutput();
  infoLine->setText(QString(data));
}

void MainWindow::addToQuickMark(const QString& string) {
  displayQuickMarkBar(QFDirection::Forward);
  quickMarkWidget_.addToQuickSearch(string);
}

void MainWindow::replaceQuickMark(const QString& string) {
  quickMarkWidget_.replaceQuickSearch(string);
}

void MainWindow::appendToQuickMark(const QString& string) {
  quickMarkWidget_.appendToQuickSearch(string);
}
void MainWindow::closeTab(int index) {
  auto widget = dynamic_cast<CrawlerWidget*>(mainTabWidget_.widget(index));

  assert(widget);

  widget->stopLoading();
  mainTabWidget_.removeTab(index);
  session_->close(widget);
  delete widget;
}

void MainWindow::currentTabChanged(int index) {
  LOG(logDEBUG) << "currentTabChanged";

  if (index >= 0) {
    CrawlerWidget* crawler_widget =
        dynamic_cast<CrawlerWidget*>(mainTabWidget_.widget(index));
    signalMux_.setCurrentDocument(crawler_widget);
    quickFindMux_.registerSelector(crawler_widget);
    quickMarkMux_.registerSelector(crawler_widget);

    // New tab is set up with fonts etc...
    emit optionsChanged();

    // Update the menu bar
    updateMenuBarFromDocument(crawler_widget);

    // Update the title bar
    updateTitleBar(QString(session_->getFilename(crawler_widget).c_str()));
  } else {
    // No tab left
    signalMux_.setCurrentDocument(nullptr);
    quickFindMux_.registerSelector(nullptr);
    quickMarkMux_.registerSelector(nullptr);

    //    infoLine->hideGauge();
    infoLine->clear();

    updateTitleBar(QString());
  }
}

void MainWindow::changeQFPattern(const QString& newPattern) {
  quickFindWidget_.changeDisplayedPattern(newPattern);
}

void MainWindow::changeQMPattern(const QString& newPattern) {
  quickMarkWidget_.changeDisplayedPattern(newPattern);
}

void MainWindow::loadFileNonInteractive(const QString& file_name) {
  LOG(logDEBUG) << "loadFileNonInteractive( " << file_name.toStdString()
                << " )";

  loadFile(file_name);

  // Try to get the window to the front
  // This is a bit of a hack but has been tested on:
  // Qt 5.3 / Gnome / Linux
  // Qt 4.8 / Win7
#ifdef _WIN32
  // Hack copied from http://qt-project.org/forums/viewthread/6164
  ::SetWindowPos((HWND)effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  ::SetWindowPos((HWND)effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#else
  Qt::WindowFlags window_flags = windowFlags();
  window_flags |= Qt::WindowStaysOnTopHint;
  setWindowFlags(window_flags);
#endif

  activateWindow();
  raise();

#ifndef _WIN32
  window_flags = windowFlags();
  window_flags &= ~Qt::WindowStaysOnTopHint;
  setWindowFlags(window_flags);
#endif

  showNormal();
}

void MainWindow::newVersionNotification(const QString& new_version) {
  LOG(logDEBUG) << "newVersionNotification( " << new_version.toStdString()
                << " )";

  QMessageBox msgBox;
  msgBox.setText(
      QString("A new version of glogg (%1) is available for download <p>"
              "<a "
              "href=\"http://glogg.bonnefon.org/download.html\">http://"
              "glogg.bonnefon.org/download.html</a>")
          .arg(new_version));
  msgBox.exec();
}

//
// Events
//

// Closes the application
void MainWindow::closeEvent(QCloseEvent* event) {
  QProcess process;
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

#ifdef _WIN32
  process.setWorkingDirectory(path);
  QString command = path + "kill-logcat.bat";
  process.startDetached(command);
#else
  process.startDetached("/bin/bash", QStringList() << path + "kill-logcat.sh");
#endif
  writeSettings();
  event->accept();
}

// Accepts the drag event if it looks like a filename
void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasFormat("text/uri-list"))
    event->acceptProposedAction();
}

// Tries and loads the file if the URL dropped is local
void MainWindow::dropEvent(QDropEvent* event) {
  foreach (const QUrl& url, event->mimeData()->urls()) {
    QString fileName = url.toLocalFile();
    if (!fileName.isEmpty()) {
      loadFile(fileName);
    }
  }
}

void MainWindow::hideTitleMenu() {
  if (mainTabWidget_.getTabBarVisibility()) {
    menuBar()->hide();
    titleBar->hide();
    mainTabWidget_.setTabBarVisibility(false);
  } else {
    menuBar()->show();
    titleBar->show();
    mainTabWidget_.setTabBarVisibility(true);
  }
}

void MainWindow::hideTab() {
  if (mainTabWidget_.getTabBarVisibility()) {
    mainTabWidget_.setTabBarVisibility(false);
  } else {
    mainTabWidget_.setTabBarVisibility(true);
  }
}

void MainWindow::startLogcat() {
  QProcess* process = new QProcess();
  //path:/home/wanghuiting1/.glogg
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
  qDebug("path:%s", path.toStdString().data());
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  QObject::connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
        emit changeFollowMode();
        process->deleteLater();
      });
#ifdef _WIN32
  LOG(logERROR) << "path: " << QDir::currentPath().toStdString();
  process->setWorkingDirectory(path);
  QString command = path + "start-logcat-pid.bat ";
  process->start(
      command, QStringList() << QDir::currentPath() << config->processFilter());
#else
  QString zipPath = config->unzipPath();
  process->start("/bin/bash", QStringList() << path + "start-logcat-pid.sh"
                                            << config->unzipPath()
                                            << config->processFilter());
#endif
}

void MainWindow::stopLogcat() {
  QProcess* process = new QProcess();
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
  QObject::connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
        emit disableFollowMode();
        process->deleteLater();
      });
#ifdef _WIN32
  process->setWorkingDirectory(path);
  QString command = path + "kill-logcat.bat";
  process->start(command);
#else
  process->start("/bin/bash", QStringList() << path + "kill-logcat.sh");
#endif
}

void MainWindow::dumpCamera() {
  QProcess* process = new QProcess();
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
//#ifdef _WIN32
//  process->setWorkingDirectory(path);
//  QString command = path + "kill-logcat.bat";
//  process->startDetached(command);
//#else
  process->startDetached("/bin/bash", QStringList() << path + "dump-camera.sh");
//#endif
}

void MainWindow::dumpStream() {
    QProcess* process = new QProcess();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    process->startDetached("/bin/bash", QStringList() << path + "dump-stream.sh");
}

void MainWindow::dumpDeviceInfo() {
    QProcess* process = new QProcess();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
    process->startDetached("/bin/bash", QStringList() << path + "dump-device-info.sh");
}

void MainWindow::fullScreen() {
  if (menuBar()->isHidden()) {
    menuBar()->show();
    titleBar->show();
    mainTabWidget_.setTabBarVisibility(true);
  } else {
    menuBar()->hide();
    titleBar->hide();
    mainTabWidget_.setTabBarVisibility(true);
  }
}

QString getSavedPath(const QString& fileName) {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  QString unzipPath = config->unzipPath();
  QString dstPath = unzipPath + QDir::separator() + fileName + ".log";
  return dstPath;
}

void MainWindow::keyPressEvent(QKeyEvent* keyEvent) {
  LOG(logDEBUG4) << "keyPressEvent received";
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
  if (keyEvent->key() == Qt::Key_Escape) {
    emit focusMainView();
    quickFindWidget_.hide();
    quickMarkWidget_.hide();
    emit hideLogBar();
    return;
  }
  // special Esc handling here
  if (keyEvent->key() == Qt::Key_F &&
      keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
    find();
  } else if (keyEvent->key() == Qt::Key_M &&
             keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
    displayQuickMarkBar(QFDirection::Forward);
  } else if (keyEvent->key() == Qt::Key_Period &&
             keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
    dropSearchBar();
  } else if (keyEvent->key() == Qt::Key_F1) {
    startLogcat();
  } else if (keyEvent->key() == Qt::Key_F2) {
    stopLogcat();
  }

  switch ((keyEvent->text())[0].toLatin1()) {
#ifdef _WIN32
    case 'T': {
      if (transparent_ != 255) {
        transparent_ = 255;
      } else {
        std::shared_ptr<Configuration> config =
            Persistent<Configuration>("settings");
        transparent_ = config->transparent();
      }
      app_->setStyle(new DarkStyle(transparent_));
      emit optionsChanged();
      break;
    }
#endif
    case 'f':
      followAction->setChecked(!followAction->isChecked());
      break;
    case 'F': {
      QWidget* parent = parentWidget();
      if (parent) parent = parent->parentWidget();
      if (parent) parent = parent->parentWidget();
      if (parent) {
        if (parent->windowState().testFlag(Qt::WindowNoState)) {
          parent->setWindowState(Qt::WindowMaximized);
        } else if (parent->windowState().testFlag(Qt::WindowMaximized)) {
          parent->setWindowState(Qt::WindowNoState);
          parent->hide();
          parent->show();
        }
      }
    } break;
    case '/':
      displayQuickFindBar(QFDirection::Forward);
      break;
    case 'A': {
      QString dstPath = getSavedPath("a");
      saveAs(dstPath);
    } break;
    case 'B': {
      std::shared_ptr<Configuration> config =
          Persistent<Configuration>("settings");
      QString aPath = getSavedPath("a");
      QString bPath = getSavedPath("b");
      saveAs(bPath);
      QProcess process;
#ifdef _WIN32
      process.setWorkingDirectory(path);
      QString command = path + "compare-log.bat ";
      process.startDetached(command, QStringList() << aPath << bPath);
#else
      process.startDetached("/bin/bash", QStringList()
                                             << path + "compare-log.sh" << aPath
                                             << bPath);
#endif

    } break;
    case '.':
      emit focusFilterBar();
      break;
    case '?':
      emit
      focusLogBar();
      break;
    default:
      keyEvent->ignore();
  }

  if (!keyEvent->isAccepted()) QMainWindow::keyPressEvent(keyEvent);
}

void MainWindow::changeFollowMode() {
  changeFollowMode(true);
  emit startNewSearch();
}

void MainWindow::disableFollowMode() {
  changeFollowMode(false);
  emit startNewSearch();
}

//
// Private functions
//

// Create a CrawlerWidget for the passed file, start its loading
// and update the title bar.
// The loading is done asynchronously.
bool MainWindow::loadFile(const QString& fileName) {
  if (fileName.endsWith(".zip") || fileName.endsWith(".tar.gz") ||
      fileName.endsWith(".gz") || fileName.endsWith(".tar") ||
      fileName.endsWith(".rar") || fileName.endsWith(".7z")) {
    process_ = new QProcess();
    QString path =
        QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    QString unzipPath = config->unzipPath();
#ifdef _WIN32
    QDir dir = QDir(QCoreApplication::applicationDirPath());
    LOG(logERROR) << "path: " << QDir::currentPath().toStdString();
    process.setWorkingDirectory(path);
    QString command = path + "open-bugreport.bat ";
    unzipPath = unzipPath + QDir::separator() + QFileInfo(fileName).baseName();
    process.startDetached(command, QStringList()
                                       << dir.toNativeSeparators(unzipPath)
                                       << dir.toNativeSeparators(fileName)
                                       << QDir::currentPath());
#else
    //    QObject::connect(process, &QProcess::readyRead, [process]() {
    //      QByteArray a = process->readAll();
    //      LOG(logDEBUG) << a.toStdString();
    //    });
    //        QObject::connect(process, SIGNAL(readyReadStdError()), this,
    //                         SLOT(updateInfoLine(process)));
    //        updateInfoLine1();
    QObject::connect(process_, SIGNAL(readyReadStandardOutput()), this,
                     SLOT(updateInfoLine1()));

    QObject::connect(
        process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus /*exitStatus*/) {
          infoLine->setVisible(false);
          process_->deleteLater();
        });
    infoLine->setVisible(true);
    process_->start("/bin/bash", QStringList() << path + "open-bugreport.sh"
                                               << fileName << unzipPath);
#endif
  } else {
    LOG(logDEBUG) << "loadFile ( " << fileName.toStdString() << " )";

    // First check if the file is already open...
    CrawlerWidget* existing_crawler = dynamic_cast<CrawlerWidget*>(
        session_->getViewIfOpen(fileName.toStdString()));
    if (existing_crawler) {
      // ... and switch to it.
      mainTabWidget_.setCurrentWidget(existing_crawler);

      return true;
    }

    // Load the file
    loadingFileName = fileName;

    try {
      CrawlerWidget* crawler_widget =
          dynamic_cast<CrawlerWidget*>(session_->open(
              fileName.toStdString(), []() { return new CrawlerWidget(); }));
      assert(crawler_widget);

      // We won't show the widget until the file is fully loaded
      crawler_widget->hide();

      // We disable the tab widget to avoid having someone switch
      // tab during loading. (maybe FIXME)

      int index = mainTabWidget_.addTab(crawler_widget, strippedName(fileName),
                                        fileName);

      // Setting the new tab, the user will see a blank page for the duration
      // of the loading, with no way to switch to another tab
      mainTabWidget_.setCurrentIndex(index);

      // Update the recent files list
      // (reload the list first in case another glogg changed it)
      GetPersistentInfo().retrieve("recentFiles");
      recentFiles_->addRecent(fileName);
      GetPersistentInfo().save("recentFiles");
      updateRecentFileActions();
    } catch (FileUnreadableErr) {
      LOG(logDEBUG) << "Can't open file " << fileName.toStdString();
      return false;
    }

    LOG(logDEBUG) << "Success loading file " << fileName.toStdString();
    return true;
  }
}

// Strips the passed filename from its directory part.
QString MainWindow::strippedName(const QString& fullFileName) const {
  return QFileInfo(fullFileName).fileName();
}

QString MainWindow::currentPath(const QString& fileName) const {
  QString current_file = session_->getFilename(currentCrawlerWidget()).c_str();
  QString dstPath = QFileInfo(current_file).absoluteDir().path() +
                    QDir::separator() + fileName + ".log";
  return dstPath;
}

// Return the currently active CrawlerWidget, or NULL if none
CrawlerWidget* MainWindow::currentCrawlerWidget() const {
  auto current = dynamic_cast<CrawlerWidget*>(mainTabWidget_.currentWidget());

  return current;
}

// Update the title bar.
void MainWindow::updateTitleBar(const QString& file_name) {
  QString shownName = tr("Untitled");
  if (!file_name.isEmpty()) shownName = strippedName(file_name);

  setWindowTitle(tr("%1 - %2").arg(shownName).arg(tr("glogg"))
#ifdef GLOGG_COMMIT
                 + " (dev build " GLOGG_VERSION ")"
#endif
  );
}

// Updates the actions for the recent files.
// Must be called after having added a new name to the list.
void MainWindow::updateRecentFileActions() {
  QStringList recent_files = recentFiles_->recentFiles();

  for (int j = 0; j < MaxRecentFiles; ++j) {
    if (j < recent_files.count()) {
      QString text = tr("&%1 %2").arg(j + 1).arg(strippedName(recent_files[j]));
      recentFileActions[j]->setText(text);
      recentFileActions[j]->setToolTip(recent_files[j]);
      recentFileActions[j]->setData(recent_files[j]);
      recentFileActions[j]->setVisible(true);
    } else {
      recentFileActions[j]->setVisible(false);
    }
  }
}

// Update our menu bar to match the settings of the crawler
// (used when the tab is changed)
void MainWindow::updateMenuBarFromDocument(const CrawlerWidget* crawler) {
  auto encoding = crawler->encodingSetting();
  encodingAction[static_cast<int>(encoding)]->setChecked(true);
  bool follow = crawler->isFollowEnabled();
  followAction->setChecked(follow);
}

// Update the top info line from the session
void MainWindow::updateInfoLine() {
  QLocale defaultLocale;

  // Following should always work as we will only receive enter
  // this slot if there is a crawler connected.
  QString current_file = session_->getFilename(currentCrawlerWidget()).c_str();

  uint64_t fileSize;
  uint32_t fileNbLine;
  QDateTime lastModified;

  session_->getFileInfo(currentCrawlerWidget(), &fileSize, &fileNbLine,
                        &lastModified);
  if (lastModified.isValid()) {
    const QString date =
        defaultLocale.toString(lastModified, QLocale::NarrowFormat);
    //    infoLine->setText(tr("%1 (%2 - %3 lines - modified on %4 - %5)")
    //                          .arg(current_file)
    //                          .arg(readableSize(fileSize))
    //                          .arg(fileNbLine)
    //                          .arg(date)
    //                          .arg(currentCrawlerWidget()->encodingText()));
  } else {
    //    infoLine->setText(tr("%1 (%2 - %3 lines - %4)")
    //                          .arg(current_file)
    //                          .arg(readableSize(fileSize))
    //                          .arg(fileNbLine)
    //                          .arg(currentCrawlerWidget()->encodingText()));
  }
}

// Write settings to permanent storage
void MainWindow::writeSettings() {
  // Save the session
  // Generate the ordered list of widgets and their topLine
  std::vector<std::tuple<const ViewInterface*, uint64_t,
                         std::shared_ptr<const ViewContextInterface>>>
      widget_list;
  for (int i = 0; i < mainTabWidget_.count(); ++i) {
    auto view = dynamic_cast<const ViewInterface*>(mainTabWidget_.widget(i));
    widget_list.push_back(std::make_tuple(view, 0UL, view->context()));
  }
  session_->save(widget_list, saveGeometry());

  // User settings
  GetPersistentInfo().save(QString("settings"));
}

// Read settings from permanent storage
void MainWindow::readSettings() {
  // Get and restore the session
  // GetPersistentInfo().retrieve( QString( "session" ) );
  // SessionInfo session = Persistent<SessionInfo>( "session" );
  /*
   * FIXME: should be in the session
  crawlerWidget->restoreState( session.crawlerState() );
  */

  // History of recent files
  GetPersistentInfo().retrieve(QString("recentFiles"));
  updateRecentFileActions();
  GetPersistentInfo().retrieve(QString("filterSet"));
  GetPersistentInfo().retrieve(QString("frqFilterSet"));
  GetPersistentPattern().retrieve(QString("sharedFilterSet"));

}

void MainWindow::displayQuickFindBar(QFDirection direction) {
  LOG(logERROR) << "MainWindow::displayQuickFindBar";

  // Warn crawlers so they can save the position of the focus in order
  // to do incremental search in the right view.
  emit enterQuickFind();

  quickFindMux_.setDirection(direction);
  quickFindWidget_.userActivate();
}

void MainWindow::displayQuickMarkBar(QFDirection direction) {
  LOG(logERROR) << "MainWindow::displayQuickMarkBar";

  // Warn crawlers so they can save the position of the focus in order
  // to do incremental search in the right view.
  emit enterQuickMark();

  quickMarkMux_.setDirection(direction);
  quickMarkWidget_.userActivate();
}

// Returns the size in human readable format
static QString readableSize(qint64 size) {
  static const QString sizeStrs[] = {QObject::tr("B"), QObject::tr("KiB"),
                                     QObject::tr("MiB"), QObject::tr("GiB"),
                                     QObject::tr("TiB")};

  QLocale defaultLocale;
  unsigned int i;
  double humanSize = size;

  for (i = 0; i + 1 < (sizeof(sizeStrs) / sizeof(QString)) &&
              (humanSize / 1024.0) >= 1024.0;
       i++)
    humanSize /= 1024.0;

  if (humanSize >= 1024.0) {
    humanSize /= 1024.0;
    i++;
  }

  QString output;
  if (i == 0)
    // No decimal part if we display straight bytes.
    output = defaultLocale.toString((int)humanSize);
  else
    output = defaultLocale.toString(humanSize, 'f', 1);

  output += QString(" ") + sizeStrs[i];

  return output;
}
