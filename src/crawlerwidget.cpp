/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015 Nicolas Bonnefon and
 * other contributors
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

// This file implements the CrawlerWidget class.
// It is responsible for creating and managing the two views and all
// the UI elements.  It implements the connection between the UI elements.
// It also interacts with the sets of data (full and filtered).

#include "log.h"

#include <cassert>

#include <frqfilterset.h>
#include <frqframe.h>
#include <QApplication>
#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QListView>
#include <QProcess>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <Qt>

#include "crawlerwidget.h"

#include "configuration.h"
#include "infoline.h"
#include "overview.h"
#include "persistentinfo.h"
#include "persistentpattern.h"
#include "pinedbutton.h"
#include "poplistview.h"
#include "qglobal.h"
#include "quickfindpattern.h"
#include "quickfindwidget.h"
#include "savedpatterns.h"
#include "savedsearches.h"

// Palette for error signaling (yellow background)
const QPalette CrawlerWidget::errorPalette(QColor("yellow"));

// Implementation of the view context for the CrawlerWidget
class CrawlerWidgetContext : public ViewContextInterface {
 public:
  // Construct from the stored string representation
  CrawlerWidgetContext(const char* string);
  // Construct from the value passsed
  CrawlerWidgetContext(QList<int> sizes, bool ignore_case, bool auto_refresh,
                       bool follow_file)
      : sizes_(sizes),
        ignore_case_(ignore_case),
        auto_refresh_(auto_refresh),
        follow_file_(follow_file) {}

  // Implementation of the ViewContextInterface function
  std::string toString() const;

  // Access the Qt sizes array for the QSplitter
  QList<int> sizes() const { return sizes_; }

  bool ignoreCase() const { return ignore_case_; }
  bool autoRefresh() const { return auto_refresh_; }
  bool followFile() const { return follow_file_; }

 private:
  QList<int> sizes_;

  bool ignore_case_;
  bool auto_refresh_;
  bool follow_file_;
};

// Constructor only does trivial construction. The real work is done once
// the data is attached.
CrawlerWidget::CrawlerWidget(QWidget* parent) : QSplitter(parent), overview_() {
  logData_ = nullptr;
  logFilteredData_ = nullptr;

  quickFindPattern_ = nullptr;
  quickMarkPattern_ = nullptr;
  savedSearches_ = nullptr;
  savedPatterns_ = nullptr;
  qfSavedFocus_ = nullptr;

  // Until we have received confirmation loading is finished, we
  // should consider we are loading something.
  loadingInProgress_ = true;
  // and it's the first time
  firstLoadDone_ = false;
  nbMatches_ = 0;
  dataStatus_ = DataStatus::OLD_DATA;

  currentLineNumber_ = 1;
  currentSearchIndex_ = -1;

  setHandleWidth(2);
}

// The top line is first one on the main display
int CrawlerWidget::getTopLine() const { return logMainView->getTopLine(); }

QString CrawlerWidget::getSelectedText() const {
  if (filteredView->hasFocus())
    return filteredView->getSelection();
  else
    return logMainView->getSelection();
}

QString CrawlerWidget::getSelectedTextWithColor() const {
  if (filteredView->hasFocus())
    return filteredView->getSelectionWithColor();
  else
    return logMainView->getSelectionWithColor();
}

void CrawlerWidget::selectAll() { activeView()->selectAll(); }

Encoding CrawlerWidget::encodingSetting() const { return encodingSetting_; }

bool CrawlerWidget::isFollowEnabled() const {
  return logMainView->isFollowEnabled();
}

QString CrawlerWidget::encodingText() const { return encoding_text_; }

// Return a pointer to the view in which we should do the QuickFind
SearchableWidgetInterface* CrawlerWidget::doGetActiveSearchable() const {
  return activeView();
}
MarkableWidgetInterface* CrawlerWidget::doGetActiveMarkable() const {
  return activeView();
}

// Return all the searchable widgets (views)
std::vector<QObject*> CrawlerWidget::doGetAllSearchables() const {
  std::vector<QObject*> searchables = {logMainView, filteredView};

  return searchables;
}

std::vector<QObject*> CrawlerWidget::doGetAllMarkables() const {
  std::vector<QObject*> markables = {logMainView, filteredView};

  return markables;
}

// Update the state of the parent
void CrawlerWidget::doSendAllStateSignals() {
  emit updateLineNumber(currentLineNumber_);
  if (!loadingInProgress_) emit loadingFinished(LoadingStatus::Successful);
}

void CrawlerWidget::updateSearchPattern(int patternIndex) {
  currentSearchIndex_ = -1;
  QString filter = "";
  currentSearchTitle_ = "";
  replaceQuickMark("");
  int index = 0;
  for (int i = 0; i < buttonList_.size(); i++) {
    index = buttonList_.at(i)->pinedIndex();
    if (buttonList_[i]->getPressed()) {
      frqFilterSet->frqFilterList[index].setEnabled(true);
      QString text = frqFilterSet->getPinedFilters(index);
      if (filter.isEmpty()) {
        filter = text;
      } else {
        filter += "|" + text;
      }
    } else {
      frqFilterSet->frqFilterList[index].setEnabled(false);
    }
  }
  *(Persistent<FrqFilterSet>("frqFilterSet")) = *frqFilterSet;
  GetPersistentInfo().save("frqFilterSet");
  currentSearchString_ = filter;
  startNewSearch(filter);
}

void CrawlerWidget::doSearch(int patternIndex) {
  QString filter = "";
  if (currentSearchIndex_ != patternIndex) {
    currentSearchIndex_ = patternIndex;
    QString text = frqFilterSet->getPinedFilters(patternIndex);
    QStringList list = text.split('>');
    if (list.length() <= 1) {
      filter = text;
    } else {
      currentSearchTitle_ = list[0].trimmed();
      filter = list[1].trimmed();

      if (list.length() >= 3) {
        replaceQuickMark(list[2].trimmed());
      }
    }
  } else {
    currentSearchIndex_ = -1;
    currentSearchTitle_ = "";
    currentSearchColor_ = "color:green";
    replaceQuickMark("");
  }

  startNewSearch(filter);
}

void CrawlerWidget::keyPressEvent(QKeyEvent* keyEvent) {
  const auto mod = keyEvent->modifiers();
  bool noModifier = mod == Qt::NoModifier;
  if (keyEvent->key() == Qt::Key_Z &&
      keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
    int min, max;
    getRange(1, &min, &max);
    viewSizeMax_ = max;
    LOG(logINFO) << "CrawlerWidget::changeTopViewSize " << sizes()[0] << " "
                 << min << " " << max;
    if (sizes()[0] == 0) {
      moveSplitter(closestLegalPosition(max / 2, 1), 1);
      setHandleWidth(2);
      searchLineEdit->show();
      lineB->show();
    } else {
      moveSplitter(closestLegalPosition(0, 1), 1);
      setHandleWidth(0);
      searchLineEdit->hide();
      lineB->hide();
    }
    LOG(logINFO) << "CrawlerWidget::changeTopViewSize " << sizes()[0];
    emit logMainView->exitView();
  } else if (keyEvent->key() == Qt::Key_V && noModifier) {
    visibilityBox->setCurrentIndex((visibilityBox->currentIndex() + 1) %
                                   visibilityBox->count());

  } else {
    const char character = (keyEvent->text())[0].toLatin1();

    if (character == '-') {
      changeTopViewSize(5);
    } else if (character == 'u') {
      moveSplitter(closestLegalPosition(viewSizeMax_ / 2, 1), 1);
    } else if (character == '=')
      changeTopViewSize(-5);
    else if (character == 'c') {
      forceToggleButton(0);
    } else if (character == 'e') {
      forceToggleButton(1);
    } else if (character == 'r') {
      resetButton(-1);
    }
    // Alt + numbers
    else if (mod == Qt::AltModifier &&
             (character >= Qt::Key_1 && character <= Qt::Key_9)) {
      int new_index = character - Qt::Key_0;
      forceToggleButton(new_index - 1);
    } else if (mod == Qt::ControlModifier && (keyEvent->key() == Qt::Key_C)) {
      emit copyToClipboard();
    } else if (mod == Qt::ControlModifier && (keyEvent->key() == Qt::Key_O)) {
      emit openFile();
    } else if (mod == Qt::ControlModifier && (keyEvent->key() == Qt::Key_A)) {
      selectAll();
    } else if (mod == Qt::ControlModifier && (keyEvent->key() == Qt::Key_Q)) {
      emit exitApp();
    }

    else
      QSplitter::keyPressEvent(keyEvent);
  }
}

void CrawlerWidget::forceToggleButton(int index) {
  if (index < buttonList_.size()) {
    resetButtonWithoutSearch(index);
    buttonList_[index]->onClick();
  }
}

void CrawlerWidget::resetButtonWithoutSearch(int except) {
  mutex.lock();
  for (int i = 0; i < buttonList_.size(); i++) {
    PinedButton* button = buttonList_[i];
    if (i != except && button->getPressed()) {
      buttonList_[i]->setPressed(false);
      buttonList_[i]->setStyleSheet("");
      int index = buttonList_.at(i)->pinedIndex();
      frqFilterSet->frqFilterList[index].setEnabled(false);
    }
  }

  *(Persistent<FrqFilterSet>("frqFilterSet")) = *frqFilterSet;
  GetPersistentInfo().save("frqFilterSet");

  if (except >= 0) {
  } else {
    currentSearchString_ = "";
    startNewSearch();
  }
  mutex.unlock();
}

void CrawlerWidget::resetButton(int except) {
  mutex.lock();
  for (int i = 0; i < buttonList_.size(); i++) {
    PinedButton* button = buttonList_[i];
    if (i != except && button->getPressed()) {
      buttonList_[i]->setPressed(false);
      buttonList_[i]->setStyleSheet("");
      int index = buttonList_.at(i)->pinedIndex();
      frqFilterSet->frqFilterList[index].setEnabled(false);
    }
  }
  *(Persistent<FrqFilterSet>("frqFilterSet")) = *frqFilterSet;
  GetPersistentInfo().save("frqFilterSet");
  if (except >= 0) {
    doSearch(except);
  } else {
    currentSearchString_ = "";
    startNewSearch();
  }
  mutex.unlock();
}

//
// Public slots
//

void CrawlerWidget::stopLoading() {
  logFilteredData_->interruptSearch();
  logData_->interruptLoading();
}

void CrawlerWidget::setT() { filteredView->updateData(); }

void CrawlerWidget::reload() {
  searchState_.resetState();
  logFilteredData_->clearSearch();
  logFilteredData_->clearMarks();
  filteredView->updateData();
  printSearchInfoMessage();

  logData_->reload();

  // A reload is considered as a first load,
  // this is to prevent the "new data" icon to be triggered.
  firstLoadDone_ = false;
}

void CrawlerWidget::setEncoding(Encoding encoding) {
  encodingSetting_ = encoding;
  updateEncoding();

  update();
}

//
// Protected functions
//
void CrawlerWidget::doSetData(std::shared_ptr<LogData> log_data,
                              std::shared_ptr<LogFilteredData> filtered_data) {
  logData_ = log_data.get();
  logFilteredData_ = filtered_data.get();
}

void CrawlerWidget::doSetQuickFindPattern(
    std::shared_ptr<QuickFindPattern> qfp) {
  quickFindPattern_ = qfp;
}

void CrawlerWidget::doSetQuickMarkPattern(
    std::shared_ptr<QuickFindPattern> qfp) {
  quickMarkPattern_ = qfp;
}

void CrawlerWidget::doSetSavedSearches(
    std::shared_ptr<SavedSearches> saved_searches) {
  savedSearches_ = saved_searches;

  // We do setup now, assuming doSetData has been called before
  // us, that's not great really...
}

void CrawlerWidget::doSetSavedPatterns(
    std::shared_ptr<SavedPatterns> saved_patterns) {
  savedPatterns_ = saved_patterns;
  setup();
}

void CrawlerWidget::doSetViewContext(const char* view_context) {
  CrawlerWidgetContext context = {view_context};

  setSizes(context.sizes());
  ignoreCaseCheck->setCheckState(context.ignoreCase() ? Qt::Checked
                                                      : Qt::Unchecked);

  auto auto_refresh_check_state =
      context.autoRefresh() ? Qt::Checked : Qt::Unchecked;
  searchRefreshCheck->setCheckState(auto_refresh_check_state);
  // Manually call the handler as it is not called when changing the state
  // programmatically
  searchRefreshChangedHandler(auto_refresh_check_state);

  LOG(logERROR) << "CrawlerWidget::doSetViewContext: " << view_context
                << " Follow File:" << context.followFile();
  emit followSet(context.followFile());
}

std::shared_ptr<const ViewContextInterface> CrawlerWidget::doGetViewContext()
    const {
  auto context = std::make_shared<const CrawlerWidgetContext>(
      sizes(), (ignoreCaseCheck->checkState() == Qt::Checked),
      (searchRefreshCheck->checkState() == Qt::Checked),
      logMainView->isFollowEnabled());

  return static_cast<std::shared_ptr<const ViewContextInterface>>(context);
}

//
// Slots
//
void CrawlerWidget::startNewSearch() {
  QString filter = "";
  startNewSearch(filter);
}

void CrawlerWidget::startNewSearch(QString& filter) {
  stopSearch();
  // Record the search line in the recent list
  // (reload the list first in case another glogg changed it)
  if (filter.isEmpty()) {
    GetPersistentInfo().retrieve("savedSearches");
    savedSearches_->addRecent(searchLineEdit->currentText());
    GetPersistentInfo().save("savedSearches");
    // Update the SearchLine (history)
    updateSearchCombo();
  }

  // Call the private function to do the search
  QString currentText = searchLineEdit->currentText();
  QString searchText = filter.isEmpty() ? currentText : "";
  if (!filter.isEmpty()) {
    if (searchText.isEmpty()) {
      searchText = filter;
    } else {
      searchText = filter + "|" + searchText;
    }
  } else {
    if (!currentSearchString_.isEmpty()) {
      if (searchText.isEmpty()) {
        searchText = currentSearchString_;
      } else {
        searchText = currentSearchString_ + "|" + searchText;
      }
    }
  }
  replaceCurrentSearch(searchText);
  filteredView->setFocus();
  logMainView->updateData();
  filteredView->updateData();
}

void CrawlerWidget::saveNewPattern() {
  GetPersistentPattern().retrieve("savedPatterns");
  savedPatterns_->addRecent(patternLineEdit->currentText());
  GetPersistentPattern().save("savedPatterns");
  patternLineEdit->clear();
  patternLineEdit->addItems(savedPatterns_->recentPatterns());
  patternLineEdit->lineEdit()->clear();
}

void CrawlerWidget::stopSearch() {
  logFilteredData_->interruptSearch();
  searchState_.stopSearch();
  printSearchInfoMessage();
}

// When receiving the 'newDataAvailable' signal from LogFilteredData
void CrawlerWidget::updateFilteredView(int nbMatches, int progress,
                                       qint64 initial_position) {
  LOG(logDEBUG) << "updateFilteredView received.";

  if (progress == 100) {
    // Searching done
    printSearchInfoMessage(nbMatches);
    searchInfoLine->hideGauge();
    // De-activate the stop button
    stopButton->setEnabled(false);
  } else {
    // Search in progress
    // We ignore 0% and 100% to avoid a flash when the search is very short
    if (progress > 0) {
      searchInfoLine->setText(
          tr("Search in progress (%1 %)... %2 match%3 found so far.")
              .arg(progress)
              .arg(nbMatches)
              .arg(nbMatches > 1 ? "es" : ""));
      searchInfoLine->displayGauge(progress);
    }
  }

  // If more (or less, e.g. come back to 0) matches have been found
  if (nbMatches != nbMatches_) {
    nbMatches_ = nbMatches;

    // Recompute the content of the filtered window.
    filteredView->updateData();

    // Update the match overview
    overview_.updateData(logData_->getNbLine());

    // New data found icon (only for "update" search)
    if (initial_position > 0) changeDataStatus(DataStatus::NEW_FILTERED_DATA);

    // Also update the top window for the coloured bullets.
    update();
  }

  // Try to restore the filtered window selection close to where it was
  // only for full searches to avoid disconnecting follow mode!
  if ((progress == 100) && (initial_position == 0) && (!isFollowEnabled())) {
    const int currenLineIndex =
        logFilteredData_->getLineIndexNumber(currentLineNumber_);
    LOG(logDEBUG) << "updateFilteredView: restoring selection: "
                  << " absolute line number (0based) " << currentLineNumber_
                  << " index " << currenLineIndex;
    filteredView->selectAndDisplayLine(currenLineIndex);
  }
  //  if (progress == 100 && isFollowEnabled()) {
  //      emit updateFocus();
  //  }
}

void CrawlerWidget::jumpToMatchingLine(int filteredLineNb) {
  int mainViewLine = logFilteredData_->getMatchingLineNumber(filteredLineNb);
  logMainView->selectAndDisplayLine(
      mainViewLine);  // FIXME: should be done with a signal.
}

void CrawlerWidget::updateLineNumberHandler(int line) {
  currentLineNumber_ = line;
  emit updateLineNumber(line);
}

void CrawlerWidget::markLineFromMain(qint64 line) {
  if (line < logData_->getNbLine()) {
    if (logFilteredData_->isLineMarked(line))
      logFilteredData_->deleteMark(line);
    else
      logFilteredData_->addMark(line);

    // Recompute the content of both window.
    filteredView->updateData();
    logMainView->updateData();

    // Update the match overview
    overview_.updateData(logData_->getNbLine());

    // Also update the top window for the coloured bullets.
    update();
  }
}

void CrawlerWidget::markLineFromMain(QList<int> lines) {
  for (int j = 0; j < lines.size(); j++) {
    markLineFromMain(lines[j]);
  }
}

void CrawlerWidget::commentLineFromMain(qint64 line, QString& commentLine) {
  QProcess process;
  QString path =
      QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
#ifdef _WIN32
  LOG(logERROR) << "path: " << QDir::currentPath().toStdString();
  process.setWorkingDirectory(path);
  QString command = path + "comment-logcat.bat";
  process.startDetached(command);
#else
  bool ok;
  QString text =
      QInputDialog::getText(this, tr("Add comment"),
                            tr("                                               "
                               "                            "),
                            QLineEdit::Normal, commentLine, &ok);
  if (ok) {
    process.startDetached(
        "/bin/bash", QStringList() << path + "comment-logcat.sh"
                                   << QVariant(line + 1).toString() << text
                                   << logData_->getFileName() << commentLine);
  }
#endif
}
void CrawlerWidget::commentLineFromFiltered(qint64 line, QString& commentLine) {
  qint64 line_in_file = logFilteredData_->getMatchingLineNumber(line);
  commentLineFromMain(line_in_file, commentLine);
}

void CrawlerWidget::markLineFromFiltered(qint64 line) {
  if (line < logFilteredData_->getNbLine()) {
    qint64 line_in_file = logFilteredData_->getMatchingLineNumber(line);
    if (logFilteredData_->filteredLineTypeByIndex(line) ==
        LogFilteredData::Mark)
      logFilteredData_->deleteMark(line_in_file);
    else
      logFilteredData_->addMark(line_in_file);

    // Recompute the content of both window.
    filteredView->updateData();
    logMainView->updateData();

    // Update the match overview
    overview_.updateData(logData_->getNbLine());

    // Also update the top window for the coloured bullets.
    update();
  }
}

void CrawlerWidget::markLineFromFiltered(QList<int> lines) {
  QList<int> mainLines;
  for (int j = 0; j < lines.size(); j++) {
    if (j < logFilteredData_->getNbLine()) {
      qint64 line_in_file = logFilteredData_->getMatchingLineNumber(lines[j]);
      mainLines.append(line_in_file);
    }
  }
  markLineFromMain(mainLines);
}

void CrawlerWidget::applyConfiguration() {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  QFont font = config->mainFont();

  LOG(logERROR) << "CrawlerWidget::applyConfiguration";

  // Whatever font we use, we should NOT use kerning
  font.setKerning(false);
  font.setFixedPitch(true);
#if QT_VERSION > 0x040700
  // Necessary on systems doing subpixel positionning (e.g. Ubuntu 12.04)
  font.setStyleStrategy(QFont::ForceIntegerMetrics);
#endif
  logMainView->setFont(font);
  filteredView->setFont(font);

  logMainView->setLineNumbersVisible(config->mainLineNumbersVisible());
  filteredView->setLineNumbersVisible(config->filteredLineNumbersVisible());

  overview_.setVisible(config->isOverviewVisible());
  logMainView->refreshOverview();

  logMainView->updateDisplaySize();
  logMainView->update();
  filteredView->updateDisplaySize();
  filteredView->update();

  // Polling interval
  logData_->setPollingInterval(
      config->pollingEnabled() ? config->pollIntervalMs() : 0);
  pollIntervalMs_ = config->pollIntervalMs();

  // Update the SearchLine (history)
  updateSearchCombo();
  updateButtons();
}
void CrawlerWidget::dropDownSearchEdit() { searchLineEdit->showPopup(); }

void CrawlerWidget::focusingFilterBar() {
  logMainView->updateData();
  filteredView->updateData();
  searchLineEdit->setFocus();
}

void CrawlerWidget::focusingLogBar() {
  patternLineEdit->show();
  patternLineEdit->setFocus();
}

void CrawlerWidget::hidingLogBar() { patternLineEdit->hide(); }

void CrawlerWidget::focusingMainView() {
  if (qfSavedFocus_ == filteredView || logMainView == qfSavedFocus_) {
    qfSavedFocus_->setFocus();
  } else {
    logMainView->setFocus();
  }
  logMainView->updateData();
  filteredView->updateData();
}

void CrawlerWidget::enteringQuickFind() {
  LOG(logERROR) << "CrawlerWidget::enteringQuickFind";

  // Remember who had the focus (only if it is one of our views)
  QWidget* focus_widget = QApplication::focusWidget();

  if ((focus_widget == logMainView) || (focus_widget == filteredView)) {
    qfSavedFocus_ = focus_widget;
  }

  else
    qfSavedFocus_ = nullptr;
}

void CrawlerWidget::exitingQuickFind() {
  // Restore the focus once the QFBar has been hidden
  if (qfSavedFocus_) qfSavedFocus_->setFocus();
}

void CrawlerWidget::enteringQuickMark() {
  LOG(logERROR) << "CrawlerWidget::enteringQuickMark";

  // Remember who had the focus (only if it is one of our views)
  QWidget* focus_widget = QApplication::focusWidget();

  if ((focus_widget == logMainView) || (focus_widget == filteredView))
    qfSavedFocus_ = focus_widget;
  else
    qfSavedFocus_ = nullptr;
}

void CrawlerWidget::exitingQuickMark() {
  // Restore the focus once the QFBar has been hidden
  if (qfSavedFocus_) qfSavedFocus_->setFocus();
}

void CrawlerWidget::loadingFinishedHandler(LoadingStatus status) {
  loadingInProgress_ = false;

  // We need to refresh the main window because the view lines on the
  // overview have probably changed.
  overview_.updateData(logData_->getNbLine());

  // FIXME, handle topLine
  // logMainView->updateData( logData_, topLine );
  logMainView->updateData();
  filteredView->updateData();
  // Shall we Forbid starting a search when loading in progress?
  // searchButton->setEnabled( false );

  // searchButton->setEnabled( true );

  // See if we need to auto-refresh the search
  if (searchState_.isAutorefreshAllowed()) {
    if (searchState_.isFileTruncated())
      // We need to restart the search
      replaceCurrentSearch(searchLineEdit->currentText());
    else
      logFilteredData_->updateSearch();
  }

  // Set the encoding for the views
  updateEncoding();

  emit loadingFinished(status);

  // Also change the data available icon
  if (firstLoadDone_) {
    changeDataStatus(DataStatus::NEW_DATA);
  } else
    firstLoadDone_ = true;
}
void CrawlerWidget::fileCommentedHandler(LogData::MonitoredFileStatus status) {
  if (status == LogData::Unchanged) {
    logData_->reload();
  }
}

void CrawlerWidget::fileChangedHandler(LogData::MonitoredFileStatus status) {
  // Handle the case where the file has been truncated
  if (status == LogData::Truncated) {
    // Clear all marks (TODO offer the option to keep them)
    logFilteredData_->clearMarks();
    if (!searchInfoLine->text().isEmpty()) {
      // Invalidate the search
      logFilteredData_->clearSearch();
      filteredView->updateData();
      searchState_.truncateFile();
      printSearchInfoMessage();
      nbMatches_ = 0;
    }
  }
}

// Returns a pointer to the window in which the search should be done
AbstractLogView* CrawlerWidget::activeView() const {
  QWidget* activeView;

  // Search in the window that has focus, or the window where 'Find' was
  // called from, or the main window.
  if (filteredView->hasFocus() || logMainView->hasFocus())
    activeView = QApplication::focusWidget();
  else
    activeView = qfSavedFocus_;

  if (activeView) {
    AbstractLogView* view = qobject_cast<AbstractLogView*>(activeView);
    return view;
  } else {
    LOG(logWARNING) << "No active view, defaulting to logMainView";
    return filteredView; /*default view */
  }
}

void CrawlerWidget::searchForward() {
  LOG(logDEBUG) << "CrawlerWidget::searchForward";

  activeView()->searchForward();
}

void CrawlerWidget::searchBackward() {
  LOG(logDEBUG) << "CrawlerWidget::searchBackward";

  activeView()->searchBackward();
}

void CrawlerWidget::markForward() {
  LOG(logDEBUG) << "CrawlerWidget::markForward";

  activeView()->markForward();
}

void CrawlerWidget::markBackward() {
  LOG(logDEBUG) << "CrawlerWidget::markBackward";

  activeView()->markBackward();
}

void CrawlerWidget::searchRefreshChangedHandler(int state) {
  searchState_.setAutorefresh(state == Qt::Checked);
  printSearchInfoMessage(logFilteredData_->getNbMatches());
}

void CrawlerWidget::searchTextChangeHandler() {
  // We suspend auto-refresh
  searchState_.changeExpression();
  printSearchInfoMessage(logFilteredData_->getNbMatches());
}

void CrawlerWidget::changeFilteredViewVisibility(int index) {
  QStandardItem* item = visibilityModel_->item(index);
  FilteredView::Visibility visibility =
      static_cast<FilteredView::Visibility>(item->data().toInt());

  filteredView->setVisibility(visibility);

  const int lineIndex =
      logFilteredData_->getLineIndexNumber(currentLineNumber_);
  filteredView->selectAndDisplayLine(lineIndex);
}

void CrawlerWidget::addToSearch(const QString& string) {
  QString text = searchLineEdit->currentText();

  if (text.isEmpty())
    text = QRegularExpression::escape(string);
  else {
    // Escape the regexp chars from the string before adding it.
    QString searchString = QRegularExpression::escape(string);
    if (!text.contains(searchString)) {
      text += ('|' + searchString);
    } else {
      text.remove(searchString + "|");
      text.remove("|" + searchString);
      text.remove(searchString);
    }
  }

  searchLineEdit->setEditText(text);

  // Set the focus to lineEdit so that the user can press 'Return' immediately
  searchLineEdit->lineEdit()->setFocus();
}

void CrawlerWidget::mouseHoveredOverMatch(qint64 line) {
  qint64 line_in_mainview = logFilteredData_->getMatchingLineNumber(line);

  overviewWidget_->highlightLine(line_in_mainview);
}

void CrawlerWidget::activityDetected() {
  changeDataStatus(DataStatus::OLD_DATA);
}

void CrawlerWidget::followModeChange(bool checked) {
  LOG(logERROR) << "followMode:" << pollIntervalMs_;
  logData_->setPollingInterval(checked ? pollIntervalMs_ : 0);
}

void CrawlerWidget::reload2() {
  filteredView->updateData();
  logMainView->updateData();
}

//
// Private functions
//

// Build the widget and connect all the signals, this must be done once
// the data are attached.
void CrawlerWidget::setup() {
  setOrientation(Qt::Vertical);

  assert(logData_);
  assert(logFilteredData_);

  // The views
  bottomWindow = new QWidget;
  overviewWidget_ = new OverviewWidget();
  logMainView =
      new LogMainView(logData_, quickFindPattern_.get(),
                      quickMarkPattern_.get(), &overview_, overviewWidget_);
  filteredView = new FilteredView(logFilteredData_, quickFindPattern_.get(),
                                  quickMarkPattern_.get());

  overviewWidget_->setOverview(&overview_);
  overviewWidget_->setParent(logMainView);
  // Default search checkboxes
  auto config = Persistent<Configuration>("settings");

  // Connect the search to the top view
  logMainView->useNewFiltering(logFilteredData_);
  QFont font = config->mainFont();

  LOG(logDEBUG) << "CrawlerWidget::applyConfiguration";

  // Whatever font we use, we should NOT use kerning
  font.setKerning(false);
  font.setFixedPitch(true);
  logMainView->setFont(font);
  filteredView->setFont(font);

  // Construct the visibility button
  visibilityModel_ = new QStandardItemModel(this);

  QStandardItem* marksAndMatchesItem =
      new QStandardItem(tr("Marks and matches"));
  QPixmap marksAndMatchesPixmap(16, 10);
  marksAndMatchesPixmap.fill(QColor(66, 66, 66, 150));
  marksAndMatchesItem->setIcon(QIcon(marksAndMatchesPixmap));
  marksAndMatchesItem->setData(FilteredView::MarksAndMatches);
  visibilityModel_->appendRow(marksAndMatchesItem);

  QStandardItem* marksItem = new QStandardItem(tr("Marks"));
  QPixmap marksPixmap(16, 10);
  marksPixmap.fill(QColor(38, 162, 193, 150));
  marksItem->setIcon(QIcon(marksPixmap));
  marksItem->setData(FilteredView::MarksOnly);
  visibilityModel_->appendRow(marksItem);

  QStandardItem* matchesItem = new QStandardItem(tr("Matches"));
  QPixmap matchesPixmap(16, 10);
  matchesPixmap.fill(QColor(170, 117, 159, 150));
  matchesItem->setIcon(QIcon(matchesPixmap));
  matchesItem->setData(FilteredView::MatchesOnly);
  visibilityModel_->appendRow(matchesItem);

  QListView* visibilityView = new QListView(this);
  visibilityView->setMovement(QListView::Static);
  visibilityView->setMinimumWidth(170);  // Only needed with custom style-sheet

  visibilityBox = new QComboBox();
  visibilityBox->setModel(visibilityModel_);
  visibilityBox->setView(visibilityView);

  // Select "Marks and matches" by default (same default as the filtered view)
  visibilityBox->setCurrentIndex(0);

  // TODO: Maybe there is some way to set the popup width to be
  // sized-to-content (as it is when the stylesheet is not overriden) in the
  // stylesheet as opposed to setting a hard min-width on the view above.
  visibilityBox->setStyleSheet(
      " \
        QComboBox:on {\
            padding: 1px 2px 1px 6px;\
            width: 19px;\
        } \
        QComboBox:!on {\
            padding: 1px 2px 1px 7px;\
            width: 19px;\
            height: 16px;\
            border: 0px;\
        } \
        QComboBox::drop-down::down-arrow {\
            width: 0px;\
            border-width: 0px;\
        } \
        QComboBox::down-arrow {\
            image: url(noimg); \
            border:0px;\
            background-position: center center;\
        }\
    ");

  // Construct the Search Info line
  searchInfoLine = new InfoLine();
  // searchInfoLine->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  searchInfoLine->setLineWidth(1);
  searchInfoLineDefaultPalette = searchInfoLine->palette();

  ignoreCaseCheck = new QCheckBox("Ignore &case");
  searchRefreshCheck = new QCheckBox("Auto-&refresh");

  // Construct the Search line
  // searchLabel = new QLabel(tr("&Text: "));

  // QFont font = searchLabel->font();
  // font.setPointSize(10);
  // searchLabel->setFont(font);

  searchLineEdit = new BoxPopupMenu;
  searchLineEdit->index = 0;
  searchLineEdit->mainMenu = searchLineEdit;
  searchLineEdit->setEditable(true);
  searchLineEdit->setCompleter(0);
  searchLineEdit->addItems(savedSearches_->recentSearches());
  searchLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  searchLineEdit->setSizeAdjustPolicy(
      QComboBox::AdjustToMinimumContentsLengthWithIcon);

  //    searchLineEdit->connect(searchLineEdit,
  //    QOverload<int>::of(&QComboBox::highlighted),
  //    QStringListModel * model = new QStringListModel;
  //    for (int x = 0; x < 1000; x++)
  //    {
  //        model->insertRow(model->rowCount());
  //        model->setData(model->index(model->rowCount() - 1, 0),
  //        QString("hello%1").arg(x), Qt::DisplayRole);
  //    }

  //    QSortFilterProxyModel * proxy = new QSortFilterProxyModel;
  //    proxy->setSourceModel(model);
  //    searchLineEdit->setModel(proxy);
  //    // When the edit text changes, use it to filter the proxy model.
  //    connect(searchLineEdit, SIGNAL(editTextChanged(QString)), proxy,
  //    SLOT(setFilterWildcard(QString)));

  QCompleter* pCompleter = new QCompleter(searchLineEdit->model(), this);
  pCompleter->setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  pCompleter->setFilterMode(Qt::MatchFlag::MatchContains);
  //    pCompleter->connect(pCompleter, QOverload<const
  //    QString&>::of(&QCompleter::highlighted),
  PopListView* listView = new PopListView();
  pCompleter->setPopup(listView);
  listView->menu = searchLineEdit;
  searchLineEdit->setCompleter(pCompleter);

  frqFrame = new FrqFrame();
  patternLineEdit = new BoxPopupMenu;
  patternLineEdit->index = 1;
  patternLineEdit->mainMenu = searchLineEdit;
  patternLineEdit->setEditable(true);
  patternLineEdit->setCompleter(0);
  patternLineEdit->addItems(savedPatterns_->recentPatterns());
  patternLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  patternLineEdit->setSizeAdjustPolicy(
      QComboBox::AdjustToMinimumContentsLengthWithIcon);
  QCompleter* pCompleter2 = new QCompleter(patternLineEdit->model(), this);
  pCompleter2->setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  pCompleter2->setFilterMode(Qt::MatchFlag::MatchContains);
  pCompleter2->connect(pCompleter2,
                       QOverload<const QString&>::of(&QCompleter::highlighted),
                       [=](const QString& text) {
                         QStringList list = text.split('>');
                         if (list.length() <= 1) {
                           patternLineEdit->hilighted = text;
                         } else {
                           patternLineEdit->hilighted = list[1].trimmed();
                           if (list.length() >= 3) {
                             patternLineEdit->marked = list[2].trimmed();
                           } else {
                             patternLineEdit->marked = "";
                           }
                         }
                       });
  PopListView* listView2 = new PopListView();
  listView2->menu = patternLineEdit;
  pCompleter2->setPopup(listView2);
  patternLineEdit->setCompleter(pCompleter2);
  patternLineEdit->hide();

  QLayout* frqFrameLayout = frqFrame->layout();

  frqFrame->setMaximumHeight(40);
  frqFrame->setContentsMargins(0, 0, 0, 0);
  frqFrameLayout->setSpacing(0);
  frqFrame->setSizePolicy(QSizePolicy::Preferred,
                          QSizePolicy::MinimumExpanding);

  //  QHBoxLayout* layout = frqFrame->horizontalLayout;
  updateButtons();

  searchButton = new QToolButton();
  searchButton->setText(tr("&Search"));
  searchButton->setAutoRaise(true);

  stopButton = new QToolButton();
  stopButton->setIcon(QIcon(":/images/stop14.png"));
  stopButton->setAutoRaise(true);
  stopButton->setEnabled(false);

  QVBoxLayout* searchLineLayout = new QVBoxLayout;
  // searchLineLayout->addWidget(searchLabel);
  searchLineLayout->addWidget(searchLineEdit);
  lineB = new QWidget;
  lineB->setFixedHeight(1);
  lineB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  lineB->setStyleSheet(QString("background-color: #424242;"));
  searchLineLayout->addWidget(lineB);
  searchLineLayout->addWidget(patternLineEdit);
  // searchLineLayout->addWidget(frqFrame);

  // searchLineLayout->addWidget(searchButton);
  // searchLineLayout->addWidget(stopButton);
  // searchLineLayout->setContentsMargins(6, 0, 6, 0);
  stopButton->setSizePolicy(
      QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  searchButton->setSizePolicy(
      QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

  QHBoxLayout* searchInfoLineLayout = new QHBoxLayout;
  searchInfoLineLayout->addWidget(visibilityBox);
  searchInfoLineLayout->addWidget(searchInfoLine);
  searchInfoLineLayout->addWidget(ignoreCaseCheck);
  searchInfoLineLayout->addWidget(searchRefreshCheck);

  // Construct the bottom window
  QVBoxLayout* bottomMainLayout = new QVBoxLayout;
  bottomMainLayout->addWidget(filteredView);
  bottomMainLayout->addLayout(searchLineLayout);
  bottomMainLayout->addWidget(frqFrame);
  bottomMainLayout->addLayout(searchInfoLineLayout);
  bottomMainLayout->setContentsMargins(0, 0, 0, 0);
  bottomMainLayout->setSpacing(0);
  bottomWindow->setLayout(bottomMainLayout);

  addWidget(logMainView);
  addWidget(bottomWindow);

  // Default splitter position (usually overridden by the config file)
  QList<int> splitterSizes;
  splitterSizes += 250;
  splitterSizes += 230;
  setSizes(splitterSizes);

  searchRefreshCheck->setCheckState(
      config->isSearchAutoRefreshDefault() ? Qt::Checked : Qt::Unchecked);
  // Manually call the handler as it is not called when changing the state
  // programmatically
  searchRefreshChangedHandler(searchRefreshCheck->checkState());
  ignoreCaseCheck->setCheckState(
      config->isSearchIgnoreCaseDefault() ? Qt::Checked : Qt::Unchecked);

  // Connect the signals
  connect(searchLineEdit->lineEdit(), SIGNAL(returnPressed()), searchButton,
          SIGNAL(clicked()));
  connect(patternLineEdit->lineEdit(), SIGNAL(returnPressed()), this,
          SLOT(saveNewPattern()));
  connect(searchLineEdit->lineEdit(), SIGNAL(textEdited(const QString&)), this,
          SLOT(searchTextChangeHandler()));
  connect(searchButton, SIGNAL(clicked()), this, SLOT(startNewSearch()));
  connect(stopButton, SIGNAL(clicked()), this, SLOT(stopSearch()));

  connect(visibilityBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(changeFilteredViewVisibility(int)));

  connect(logMainView, SIGNAL(newSelection(int)), logMainView, SLOT(update()));
  connect(filteredView, SIGNAL(newSelection(int)), this,
          SLOT(jumpToMatchingLine(int)));
  connect(filteredView, SIGNAL(newSelection(int)), filteredView,
          SLOT(update()));
  connect(logMainView, SIGNAL(updateLineNumber(int)), this,
          SLOT(updateLineNumberHandler(int)));
  connect(this, SIGNAL(updateFocus()), filteredView, SLOT(updateFocus()));
  connect(logMainView, SIGNAL(markLine(qint64)), this,
          SLOT(markLineFromMain(qint64)));
  connect(filteredView, SIGNAL(markLine(qint64)), this,
          SLOT(markLineFromFiltered(qint64)));

  connect(logMainView, SIGNAL(markLines(QList<int>)), this,
          SLOT(markLineFromMain(QList<int>)));
  connect(filteredView, SIGNAL(markLines(QList<int>)), this,
          SLOT(markLineFromFiltered(QList<int>)));

  connect(logMainView, SIGNAL(commentLine(qint64, QString&)), this,
          SLOT(commentLineFromMain(qint64, QString&)));
  connect(filteredView, SIGNAL(commentLine(qint64, QString&)), this,
          SLOT(commentLineFromFiltered(qint64, QString&)));

  connect(logMainView, SIGNAL(changeFollowMode()), this,
          SIGNAL(changeFollowMode()));
  connect(filteredView, SIGNAL(changeFollowMode()), this,
          SIGNAL(changeFollowMode()));

  connect(logMainView, SIGNAL(disableFollowMode()), this,
          SIGNAL(disableFollowMode()));
  connect(filteredView, SIGNAL(disableFollowMode()), this,
          SIGNAL(disableFollowMode()));

  connect(logMainView, SIGNAL(addToSearch(const QString&)), this,
          SLOT(addToSearch(const QString&)));
  connect(filteredView, SIGNAL(addToSearch(const QString&)), this,
          SLOT(addToSearch(const QString&)));

  connect(logMainView, SIGNAL(addToQuickSearch(const QString&)), this,
          SIGNAL(addToQuickSearch(const QString&)));
  connect(filteredView, SIGNAL(addToQuickSearch(const QString&)), this,
          SIGNAL(addToQuickSearch(const QString&)));

  connect(logMainView, SIGNAL(addToQuickMark(const QString&)), this,
          SIGNAL(addToQuickMark(const QString&)));
  connect(filteredView, SIGNAL(addToQuickMark(const QString&)), this,
          SIGNAL(addToQuickMark(const QString&)));

  connect(filteredView, SIGNAL(mouseHoveredOverLine(qint64)), this,
          SLOT(mouseHoveredOverMatch(qint64)));
  connect(filteredView, SIGNAL(mouseLeftHoveringZone()), overviewWidget_,
          SLOT(removeHighlight()));

  // Follow option (up and down)
  connect(this, SIGNAL(followSet(bool)), logMainView, SLOT(followSet(bool)));
  connect(this, SIGNAL(followSet(bool)), filteredView, SLOT(followSet(bool)));
  connect(logMainView, SIGNAL(followModeChanged(bool)), this,
          SIGNAL(followModeChanged(bool)));
  connect(filteredView, SIGNAL(followModeChanged(bool)), this,
          SIGNAL(followModeChanged(bool)));

  connect(logMainView, SIGNAL(followModeChange(bool)), this,
          SLOT(followModeChange(bool)));
  connect(filteredView, SIGNAL(followModeChange(bool)), this,
          SLOT(followModeChange(bool)));

  // Detect activity in the views
  connect(logMainView, SIGNAL(activity()), this, SLOT(activityDetected()));
  connect(filteredView, SIGNAL(activity()), this, SLOT(activityDetected()));

  connect(logFilteredData_, SIGNAL(searchProgressed(int, int, qint64)), this,
          SLOT(updateFilteredView(int, int, qint64)));

  // Sent load file update to MainWindow (for status update)
  connect(logData_, SIGNAL(loadingProgressed(int)), this,
          SIGNAL(loadingProgressed(int)));
  connect(logData_, SIGNAL(loadingFinished(LoadingStatus)), this,
          SLOT(loadingFinishedHandler(LoadingStatus)));
  connect(logData_, SIGNAL(fileChanged(LogData::MonitoredFileStatus)), this,
          SLOT(fileChangedHandler(LogData::MonitoredFileStatus)));
  connect(logData_, SIGNAL(fileCommented(LogData::MonitoredFileStatus)), this,
          SLOT(fileCommentedHandler(LogData::MonitoredFileStatus)));

  // Search auto-refresh
  connect(searchRefreshCheck, SIGNAL(stateChanged(int)), this,
          SLOT(searchRefreshChangedHandler(int)));

  // Advise the parent the checkboxes have been changed
  // (for maintaining default config)
  connect(searchRefreshCheck, SIGNAL(stateChanged(int)), this,
          SIGNAL(searchRefreshChanged(int)));
  connect(ignoreCaseCheck, SIGNAL(stateChanged(int)), this,
          SIGNAL(ignoreCaseChanged(int)));

  // Switch between views
  connect(logMainView, SIGNAL(exitView()), filteredView, SLOT(setFocus()));
  connect(filteredView, SIGNAL(exitView()), logMainView, SLOT(setFocus()));
  connect(logMainView, SIGNAL(exitView()), this, SLOT(reload2()));
  connect(filteredView, SIGNAL(exitView()), this, SLOT(reload2()));
}

// Create a new search using the text passed, replace the currently
// used one and destroy the old one.
void CrawlerWidget::replaceCurrentSearch(const QString& searchText) {
  // Interrupt the search if it's ongoing
  logFilteredData_->interruptSearch();

  // We have to wait for the last search update (100%)
  // before clearing/restarting to avoid having remaining results.

  // FIXME: this is a bit of a hack, we call processEvents
  // for Qt to empty its event queue, including (hopefully)
  // the search update event sent by logFilteredData_. It saves
  // us the overhead of having proper sync.
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  nbMatches_ = 0;

  // Clear and recompute the content of the filtered window.
  logFilteredData_->clearSearch();
  filteredView->updateData();

  // Update the match overview
  overview_.updateData(logData_->getNbLine());

  if (!searchText.isEmpty()) {
    QString pattern;

    // Determine the type of regexp depending on the config
    static std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    switch (config->mainRegexpType()) {
      case FixedString:
        pattern = QRegularExpression::escape(searchText);
        break;
      default:
        pattern = searchText;
        break;
    }

    // Set the pattern case insensitive if needed
    QRegularExpression::PatternOptions patternOptions =
        QRegularExpression::UseUnicodePropertiesOption |
        QRegularExpression::OptimizeOnFirstUsageOption;

    if (ignoreCaseCheck->checkState() == Qt::Checked)
      patternOptions |= QRegularExpression::CaseInsensitiveOption;

    // Constructs the regexp
    QRegularExpression regexp(pattern, patternOptions);

    if (regexp.isValid()) {
      // Activate the stop button
      stopButton->setEnabled(true);
      // Start a new asynchronous search
      logFilteredData_->runSearch(regexp);
      // Accept auto-refresh of the search
      searchState_.startSearch();
    } else {
      // The regexp is wrong
      logFilteredData_->clearSearch();
      filteredView->updateData();
      searchState_.resetState();

      // Inform the user
      QString errorMessage = tr("Error in expression");
      const int offset = regexp.patternErrorOffset();
      if (offset != -1) {
        errorMessage += " at position ";
        errorMessage += QString::number(offset);
      }
      errorMessage += ": ";
      errorMessage += regexp.errorString();
      searchInfoLine->setPalette(errorPalette);
      searchInfoLine->setText(errorMessage);
    }
  } else {
    searchState_.resetState();
    printSearchInfoMessage();
  }
}
void CrawlerWidget::updateButtons() {
  QLayout* frqFrameLayout = frqFrame->layout();
  frqFrameLayout->removeWidget(scrollArea_);
  for (int i = 0; i < buttonList_.size(); i++) {
    pinnedPatternsLayout_->removeWidget(buttonList_[i]);
    delete buttonList_[i];
  }

  scrollArea_ = new QScrollArea(this);
  scrollArea_->setWidgetResizable(true);
  scrollArea_->setSizePolicy(QSizePolicy::Preferred,
                             QSizePolicy::MinimumExpanding);
  scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea_->setContentsMargins(0, 0, 0, 0);

  container_ = new QWidget();
  scrollArea_->setWidget(container_);
  pinnedPatternsLayout_ = new QHBoxLayout(container_);

  container_->setSizePolicy(QSizePolicy::Preferred,
                            QSizePolicy::MinimumExpanding);
  pinnedPatternsLayout_->setSpacing(0);
  pinnedPatternsLayout_->setContentsMargins(0, 0, 0, 9);

  GetPersistentInfo().retrieve("frqFilterSet");
  frqFilterSet = PersistentCopy<FrqFilterSet>("frqFilterSet");

  buttonList_.clear();
  for (int i = 0; i < frqFilterSet->getSize(); i++) {
    QColor foreColor, backColor;
    PinedButton* b = new PinedButton();

    QString text = frqFilterSet->getPinedDescription(i);

    b->setText(text);
    pinnedPatternsLayout_->addWidget(b);
    b->setPinedIndex(i);
    b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    frqFilterSet->getPinedFiltersColor(i, &foreColor, &backColor);
    b->setColor(backColor, foreColor);
    b->setAutoFillBackground(true);

    connect(b, SIGNAL(clicked()), b, SLOT(onClick()));
    connect(b, SIGNAL(clicked(int)), this, SLOT(updateSearchPattern(int)));
    connect(b, SIGNAL(removed(int)), this, SLOT(updateSearchPattern(int)));
    buttonList_.append(b);
  }
  pinnedPatternsLayout_->addStretch(1);

  frqFrameLayout->addWidget(scrollArea_);
}

// Updates the content of the drop down list for the saved searches,
// called when the SavedSearch has been changed.
void CrawlerWidget::updateSearchCombo() {
  const QString text = searchLineEdit->lineEdit()->text();
  searchLineEdit->clear();
  searchLineEdit->addItems(savedSearches_->recentSearches());
  // In case we had something that wasn't added to the list (blank...):
  searchLineEdit->lineEdit()->setText(text);
}

// Print the search info message.
void CrawlerWidget::printSearchInfoMessage(int nbMatches) {
  QString text;

  switch (searchState_.getState()) {
    case SearchState::NoSearch:
      // Blank text is fine
      break;
    case SearchState::Static:
      text =
          tr("%1 match%2 found.").arg(nbMatches).arg(nbMatches > 1 ? "es" : "");
      break;
    case SearchState::Autorefreshing:
      text = tr("%1 match%2 found. Search is auto-refreshing...")
                 .arg(nbMatches)
                 .arg(nbMatches > 1 ? "es" : "");
      break;
    case SearchState::FileTruncated:
    case SearchState::TruncatedAutorefreshing:
      text =
          tr("File truncated on disk, previous search results are not "
             "valid anymore.");
      break;
  }
  if (!currentSearchTitle_.isEmpty()) {
    text = "<span style=\"" + currentSearchColor_ + "\">Searching " +
           currentSearchTitle_ + "...</span>" + text;
  }

  searchInfoLine->setPalette(searchInfoLineDefaultPalette);
  searchInfoLine->setText(text);
}

// Change the data status and, if needed, advise upstream.
void CrawlerWidget::changeDataStatus(DataStatus status) {
  if ((status != dataStatus_) &&
      (!(dataStatus_ == DataStatus::NEW_FILTERED_DATA &&
         status == DataStatus::NEW_DATA))) {
    dataStatus_ = status;
    emit dataStatusChanged(dataStatus_);
  }
}

// Determine the right encoding and set the views.
void CrawlerWidget::updateEncoding() {
  Encoding encoding = Encoding::ENCODING_MAX;

  switch (encodingSetting_) {
    case Encoding::ENCODING_AUTO:
      switch (logData_->getDetectedEncoding()) {
        case EncodingSpeculator::Encoding::ASCII7:
          encoding = Encoding::ENCODING_ISO_8859_1;
          encoding_text_ = tr("US-ASCII");
          break;
        case EncodingSpeculator::Encoding::ASCII8:
          encoding = Encoding::ENCODING_ISO_8859_1;
          encoding_text_ = tr("ISO-8859-1");
          break;
        case EncodingSpeculator::Encoding::UTF8:
          encoding = Encoding::ENCODING_UTF8;
          encoding_text_ = tr("UTF-8");
          break;
        case EncodingSpeculator::Encoding::UTF16LE:
          encoding = Encoding::ENCODING_UTF16LE;
          encoding_text_ = tr("UTF-16LE");
          break;
        case EncodingSpeculator::Encoding::UTF16BE:
          encoding = Encoding::ENCODING_UTF16BE;
          encoding_text_ = tr("UTF-16BE");
          break;
      }
      break;
    case Encoding::ENCODING_UTF8:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as UTF-8");
      break;
    case Encoding::ENCODING_UTF16LE:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as UTF-16LE");
      break;
    case Encoding::ENCODING_UTF16BE:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as UTF-16BE");
      break;
    case Encoding::ENCODING_CP1251:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as CP1251");
      break;
    case Encoding::ENCODING_CP1252:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as CP1252");
      break;
    case Encoding::ENCODING_BIG5:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as Big5");
      break;
    case Encoding::ENCODING_GB18030:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as GB18030/GB2312");
      break;
    case Encoding::ENCODING_SHIFT_JIS:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as Shift_JIS");
      break;
    case Encoding::ENCODING_KOI8R:
      encoding = encodingSetting_;
      encoding_text_ = tr("Displayed as KOI8-R");
      break;
    case Encoding::ENCODING_ISO_8859_1:
    default:
      encoding = Encoding::ENCODING_ISO_8859_1;
      encoding_text_ = tr("Displayed as ISO-8859-1");
      break;
  }

  logData_->setDisplayEncoding(encoding);
  logMainView->forceRefresh();
  logFilteredData_->setDisplayEncoding(encoding);
  filteredView->forceRefresh();
}

// Change the respective size of the two views
void CrawlerWidget::changeTopViewSize(int32_t delta) {
  int min, max;
  getRange(1, &min, &max);
  viewSizeMax_ = max;
  LOG(logINFO) << "CrawlerWidget::changeTopViewSize " << sizes()[0] << " "
               << min << " " << max;
  moveSplitter(closestLegalPosition(sizes()[0] + (delta * 10), 1), 1);
  LOG(logINFO) << "CrawlerWidget::changeTopViewSize " << sizes()[0];
}

//
// SearchState implementation
//
void CrawlerWidget::SearchState::resetState() { state_ = NoSearch; }

void CrawlerWidget::SearchState::setAutorefresh(bool refresh) {
  autoRefreshRequested_ = refresh;

  if (refresh) {
    if (state_ == Static) state_ = Autorefreshing;
    /*
    else if ( state_ == FileTruncated )
        state_ = TruncatedAutorefreshing;
    */
  } else {
    if (state_ == Autorefreshing)
      state_ = Static;
    else if (state_ == TruncatedAutorefreshing)
      state_ = FileTruncated;
  }
}

void CrawlerWidget::SearchState::truncateFile() {
  if (state_ == Autorefreshing || state_ == TruncatedAutorefreshing) {
    state_ = TruncatedAutorefreshing;
  } else {
    state_ = FileTruncated;
  }
}

void CrawlerWidget::SearchState::changeExpression() {
  if (state_ == Autorefreshing) state_ = Static;
}

void CrawlerWidget::SearchState::stopSearch() {
  if (state_ == Autorefreshing) state_ = Static;
}

void CrawlerWidget::SearchState::startSearch() {
  if (autoRefreshRequested_)
    state_ = Autorefreshing;
  else
    state_ = Static;
}

/*
 * CrawlerWidgetContext
 */
CrawlerWidgetContext::CrawlerWidgetContext(const char* string) {
  QRegularExpression regex("S(\\d+):(\\d+)");
  QRegularExpressionMatch match = regex.match(string);
  if (match.hasMatch()) {
    sizes_ = {match.captured(1).toInt(), match.captured(2).toInt()};
    LOG(logDEBUG) << "sizes_: " << sizes_[0] << " " << sizes_[1];
  } else {
    LOG(logWARNING) << "Unrecognised view size: " << string;

    // Default values;
    sizes_ = {100, 400};
  }

  QRegularExpression case_refresh_regex("IC(\\d+):AR(\\d+)");
  match = case_refresh_regex.match(string);
  if (match.hasMatch()) {
    ignore_case_ = (match.captured(1).toInt() == 1);
    auto_refresh_ = (match.captured(2).toInt() == 1);

    LOG(logDEBUG) << "ignore_case_: " << ignore_case_
                  << " auto_refresh_: " << auto_refresh_;
  } else {
    LOG(logWARNING) << "Unrecognised case/refresh: " << string;
    ignore_case_ = false;
    auto_refresh_ = false;
  }

  QRegularExpression follow_regex("FF(\\d+)");
  match = follow_regex.match(string);
  if (match.hasMatch()) {
    follow_file_ = (match.captured(1).toInt() == 1);

    LOG(logDEBUG) << "follow_file_: " << follow_file_;
  } else {
    LOG(logWARNING) << "Unrecognised follow: " << string;
    follow_file_ = false;
  }
}

std::string CrawlerWidgetContext::toString() const {
  char string[160];

  snprintf(string, sizeof string, "S%d:%d:IC%d:AR%d:FF%d", sizes_[0], sizes_[1],
           ignore_case_, auto_refresh_, follow_file_);

  return {string};
}
