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

#include <QCommandLineParser>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QTranslator>

#include <memory>

#include <frqfilterset.h>
#include <iomanip>
#include <iostream>
// Not `using namespace std;`: this file pulls in <windows.h> below, and with
// C++17 a wholesale using-directive makes the SDK's `byte` typedef ambiguous
// with std::byte. Pull in only what we use.
using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

#ifdef _WIN32
#include "unistd.h"
#endif

#include "configuration.h"
#include "filterset.h"
#include "gloggapp.h"
#include "loadingstatus.h"
#include "mainwindow.h"
#include "persistentinfo.h"
#include "persistentpattern.h"
#include "recentfiles.h"
#include "savedpatterns.h"
#include "savedsearches.h"
#include "session.h"
#include "sessioninfo.h"

#include "externalcom.h"

#ifdef GLOGG_SUPPORTS_DBUS
#include "dbusexternalcom.h"
#elif GLOGG_SUPPORTS_SOCKETIPC
#include "socketexternalcom.h"
#endif
#include "framelesswindow/framelesswindow.h"
#include "thememanager.h"

#include "log.h"
#include "sharedfilterset.h"
#include "version/versionmanager.h"

static void print_version();

int main(int argc, char* argv[]) {
  GloggApp app(argc, argv);
  vector<string> filenames;

  // Configuration
  bool new_session = false;
  bool load_session = false;
  bool multi_instance = false;
#ifdef _WIN32
  bool log_to_file = false;
#endif

  TLogLevel logLevel = logWARNING;

  // -d, -dd, ... -dddddddddd were distinct hidden options under
  // Boost.program_options. Pull them out first so QCommandLineParser never
  // sees them (it would treat -dd as -d -d).
  int debugCount = 0;
  QStringList arguments;
  {
    const QStringList allArguments = QCoreApplication::arguments();
    // QCommandLineParser::parse() expects element 0 to be the program name
    // and skips it, so keep it at the front of the list we hand over.
    if (!allArguments.isEmpty()) arguments.append(allArguments.first());
    static const QRegularExpression debugOption(
        QStringLiteral("^--debug$|^-{1,2}d{1,10}$"));
    for (const QString& argument : allArguments.mid(1)) {
      if (debugOption.match(argument).hasMatch())
        ++debugCount;
      else
        arguments.append(argument);
    }
  }

  try {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Usage: glogg [options] [files]"));
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(
        {"v", "version"}, QStringLiteral("print glogg's version information")));
    parser.addOption(QCommandLineOption(
        {"m", "multi"},
        QStringLiteral("allow multiple instance of glogg to run "
                       "simultaneously (use together with -s)")));
    parser.addOption(
        QCommandLineOption({"s", "load-session"},
                           QStringLiteral("load the previous session (default "
                                          "when no file is passed)")));
    parser.addOption(
        QCommandLineOption({"n", "new-session"},
                           QStringLiteral("do not load the previous session "
                                          "(default when a file is passed)")));
#ifdef _WIN32
    parser.addOption(QCommandLineOption(
        {"l", "log"}, QStringLiteral("save the log to a file (Windows only)")));
#endif
    parser.addOption(QCommandLineOption(
        {"d", "debug"},
        QStringLiteral("output more debug (include multiple times for more "
                       "verbosity e.g. -dddd)")));
    parser.addPositionalArgument("files",
                                 QStringLiteral("log files to open"));

    if (!parser.parse(arguments)) {
      cerr << "Option processing error: "
           << parser.errorText().toStdString() << endl;
      return 1;
    }

    if (parser.isSet("help")) {
      cout << parser.helpText().toStdString();
      return 0;
    }

    if (parser.isSet("version")) {
      print_version();
      return 0;
    }

    if (debugCount > 0) logLevel = (TLogLevel)(logWARNING + debugCount);

    if (parser.isSet("multi")) multi_instance = true;

    if (parser.isSet("new-session")) new_session = true;

    if (parser.isSet("load-session")) load_session = true;

#ifdef _WIN32
    if (parser.isSet("log")) log_to_file = true;
#endif

    for (const QString& file : parser.positionalArguments())
      filenames.push_back(file.toStdString());
  } catch (exception& e) {
    cerr << "Option processing error: " << e.what() << endl;
    return 1;
  } catch (...) {
    cerr << "Exception of unknown type!\n";
  }

#ifdef _WIN32
  if (log_to_file) {
    char file_name[255];
    snprintf(file_name, sizeof file_name, "glogg_%d.log", getpid());
    FILE* file = fopen(file_name, "w");
    Output2FILE::Stream() = file;
  }
#endif

  FILELog::setReportingLevel(logDEBUG);

  for (auto& filename : filenames) {
    if (!filename.empty()) {
      // Convert to absolute path
      QFileInfo file(QString::fromLocal8Bit(filename.c_str()));
      filename = file.absoluteFilePath().toStdString();
      LOG(logDEBUG) << "Filename: " << filename;
    }
  }

  // External communicator
  shared_ptr<ExternalCommunicator> externalCommunicator = nullptr;
  shared_ptr<ExternalInstance> externalInstance = nullptr;

  try {
#ifdef GLOGG_SUPPORTS_DBUS
    externalCommunicator = make_shared<DBusExternalCommunicator>();
    externalInstance =
        shared_ptr<ExternalInstance>(externalCommunicator->otherInstance());
#elif GLOGG_SUPPORTS_SOCKETIPC
    externalCommunicator = make_shared<SocketExternalCommunicator>();
    auto ptr = externalCommunicator->otherInstance();
    externalInstance = shared_ptr<ExternalInstance>(ptr);
#endif
  } catch (CantCreateExternalErr& e) {
    LOG(logWARNING) << "Cannot initialise external communication.";
  }

  LOG(logDEBUG) << "externalInstance = " << externalInstance;
  if ((!multi_instance) && externalInstance) {
    uint32_t version = externalInstance->getVersion();
    LOG(logINFO) << "Found another glogg (version = " << std::setbase(16)
                 << version << ")";

    for (const auto& filename : filenames) {
      externalInstance->loadFile(QString::fromStdString(filename));
    }

    return 0;
  } else {
    // FIXME: there is a race condition here. One glogg could start
    // between the declaration of externalInstance and here,
    // is it a problem?
    if (externalCommunicator) externalCommunicator->startListening();
  }

  // Register types for Qt
  qRegisterMetaType<LoadingStatus>("LoadingStatus");

  // Register the configuration items
  GetPersistentInfo().migrateAndInit();
  GetPersistentInfo().registerPersistable(std::make_shared<SessionInfo>(),
                                          QString("session"));
  GetPersistentInfo().registerPersistable(std::make_shared<Configuration>(),
                                          QString("settings"));
  GetPersistentInfo().registerPersistable(std::make_shared<FilterSet>(),
                                          QString("filterSet"));
  GetPersistentInfo().registerPersistable(std::make_shared<FrqFilterSet>(),
                                          QString("frqFilterSet"));
  GetPersistentInfo().registerPersistable(std::make_shared<SavedSearches>(),
                                          QString("savedSearches"));
  GetPersistentPattern().registerPersistable(std::make_shared<SavedPatterns>(),
                                             QString("savedPatterns"));
  GetPersistentInfo().registerPersistable(std::make_shared<RecentFiles>(),
                                          QString("recentFiles"));
  GetPersistentPattern().registerPersistable(std::make_shared<SharedFilterSet>(),
                                          QString("sharedFilterSet"));
#ifdef GLOGG_SUPPORTS_VERSION_CHECKING
  GetPersistentInfo().registerPersistable(
      std::make_shared<VersionCheckerConfig>(), QString("versionChecker"));
#endif

#ifdef _WIN32
  // Allow the app to raise it's own windows (in case an external
  // glogg send us a file to open)
  AllowSetForegroundWindow(ASFW_ANY);
#endif

  // No icon in menus
  app.setAttribute(Qt::AA_DontShowIconsInMenus);

  // FIXME: should be replaced by a two staged init of MainWindow
  GetPersistentInfo().retrieve(QString("settings"));

  // Load translation
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  QString lang = config->language();
  if (lang.isEmpty()) {
    // First run: use system locale
    lang = QLocale::system().name();
  }
  if (lang != "en") {
    QTranslator* translator = new QTranslator(&app);
    if (translator->load(":/translations/glogg_" + lang + ".qm")) {
      app.installTranslator(translator);
    }
  }

  // Load theme
  ThemeManager::instance().loadTheme(config->themePath());

  std::unique_ptr<Session> session(new Session());
  FramelessWindow framelessWindow;
  MainWindow mw(std::move(session), externalCommunicator,
                framelessWindow.getTitleBar());
  mw.setApplication(&app);
  mw.reloadGeometry();

  if (load_session ||
      (filenames.empty() && !new_session && config->loadLastSession()))
    mw.reloadSession();

  framelessWindow.setContent(&mw);
  framelessWindow.setWindowIcon(mw.getIcon());
  framelessWindow.show();

  for (const auto& filename : filenames) {
    mw.loadInitialFile(QString::fromStdString(filename));
  }

  mw.startBackgroundTasks();

  return app.exec();
}

static void print_version() {
  cout << "glogg " GLOGG_VERSION "\n";
#ifdef GLOGG_COMMIT
  cout << "Built " GLOGG_DATE " from " GLOGG_COMMIT "\n";
#endif
  cout << "Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015 Nicolas "
          "Bonnefon and other contributors\n";
  cout << "This is free software.  You may redistribute copies of it under the "
          "terms of\n";
  cout << "the GNU General Public License "
          "<http://www.gnu.org/licenses/gpl.html>.\n";
  cout << "There is NO WARRANTY, to the extent permitted by law.\n";
}
