/*
 * Copyright (C) 2013, 2014 Nicolas Bonnefon and other contributors
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

#include "log.h"

#include "configuration.h"
#include "persistentinfo.h"
#include "quickmarkmux.h"

#include "qfnotifications.h"

QuickMarkMux::QuickMarkMux(std::shared_ptr<QuickFindPattern> pattern)
    : QObject(), pattern_(pattern), registeredMarkables_() {
  selector_ = nullptr;

  // Forward the pattern's signal to our listeners
  connect(pattern_.get(), SIGNAL(patternUpdated(QList<int>)), this,
          SLOT(notifyPatternChanged(QList<int>)));
}

//
// Public member functions
//
void QuickMarkMux::registerSelector(
    const QuickMarkMuxSelectorInterface* selector) {
  LOG(logDEBUG) << "QuickMarkMux::registerSelector";

  // The selector object we will use when forwarding Mark requests
  selector_ = selector;

  unregisterAllMarkables();

  if (selector) {
    for (auto i : selector_->getAllMarkables()) registerMarkable(i);
  } else {
    // null selector, all is well, we don't do anything.
  }
}

void QuickMarkMux::setDirection(QFDirection direction) {
  LOG(logDEBUG) << "QuickMarkMux::setDirection: new direction: " << direction;
  currentDirection_ = direction;
}

//
// Public slots
//
void QuickMarkMux::markNext(QFDirection direction) {
  LOG(logDEBUG) << "QuickMarkMux::MarkNext";
  QFDirection dt =
      direction == QFDirection::UnKnown ? currentDirection_ : direction;
  if (dt == QFDirection::Forward)
    markForward();
  else
    markBackward();
}

void QuickMarkMux::markPrevious(QFDirection direction) {
  LOG(logDEBUG) << "QuickMarkMux::MarkPrevious";
  QFDirection dt =
      direction == QFDirection::UnKnown ? currentDirection_ : direction;
  if (dt == QFDirection::Forward)
    markBackward();
  else
    markForward();
}
void QuickMarkMux::markNext() { markNext(QFDirection::UnKnown); }
void QuickMarkMux::markPrevious() { markPrevious(QFDirection::UnKnown); }

void QuickMarkMux::markForward() {
  LOG(logINFO) << "QuickMarkMux::MarkForward";

  if (auto markable = getMarkableWidget()) markable->markForward();
}

void QuickMarkMux::markBackward() {
  LOG(logINFO) << "QuickMarkMux::MarkBackward";

  if (auto markable = getMarkableWidget()) markable->markBackward();
}

void QuickMarkMux::setNewPattern(const QString& new_pattern, bool ignore_case) {
  static std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");

  LOG(logINFO) << "QuickMarkMux::setNewPattern";

  // If we must do an incremental Mark, we do it now
  if (config->isQuickfindIncremental()) {
    pattern_->changeMarkPattern(new_pattern, ignore_case);
    if (auto markable = getMarkableWidget()) {
      if (currentDirection_ == QFDirection::Forward)
        markable->incrementallyMarkForward();
      else
        markable->incrementallyMarkBackward();
    }
  }
}

void QuickMarkMux::confirmPatternWithoutSearch(const QString& new_pattern,
                                               bool ignore_case) {
  static std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");

  pattern_->changeMarkPattern(new_pattern, ignore_case);
}

void QuickMarkMux::confirmPattern(const QString& new_pattern, bool ignore_case,
                                  QFDirection direction) {
  static std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");

  pattern_->changeMarkPattern(new_pattern, ignore_case);

  // if non-incremental, we perform the Mark now
  if (!config->isQuickfindIncremental()) {
    markNext(direction);
  } else {
    if (auto markable = getMarkableWidget()) markable->incrementalMarkStop();
  }
}

void QuickMarkMux::cancelMark() {
  static std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");

  if (config->isQuickfindIncremental()) {
    if (auto markable = getMarkableWidget()) markable->incrementalMarkAbort();
  }
}

//
// Private slots
//
void QuickMarkMux::changeQuickMark(const QString& new_pattern,
                                   QFDirection new_direction) {
  pattern_->changeMarkPattern(new_pattern);
  setDirection(new_direction);
}

void QuickMarkMux::notifyPatternChanged(QList<int> removedList) {
  emit patternChanged(pattern_->getPattern());
}

//
// Private member functions
//

// Use the registered 'selector' to determine where to send the Mark requests.
MarkableWidgetInterface* QuickMarkMux::getMarkableWidget() const {
  LOG(logDEBUG) << "QuickMarkMux::getMarkableWidget";

  MarkableWidgetInterface* markable = nullptr;

  if (selector_)
    markable = selector_->getActiveMarkable();
  else
    LOG(logWARNING)
        << "QuickMarkMux::getActiveMarkable() no registered selector";

  return markable;
}

void QuickMarkMux::registerMarkable(QObject* markable) {
  LOG(logDEBUG) << "QuickMarkMux::registerMarkable";

  // The Markable can change our qf pattern
  connect(markable, SIGNAL(changeQuickMark(const QString&, QFDirection)), this,
          SLOT(changeQuickMark(const QString&, QFDirection)));
  // Send us notifications
  connect(markable, SIGNAL(notifyQuickMark(const QFNotification&)), this,
          SIGNAL(notify(const QFNotification&)));

  // And clear them
  connect(markable, SIGNAL(clearQuickMarkNotification()), this,
          SIGNAL(clearNotification()));
  // Mark can be initiated by the view itself
  connect(markable, SIGNAL(markNext()), this, SLOT(markNext()));
  connect(markable, SIGNAL(markPrevious()), this, SLOT(markPrevious()));

  registeredMarkables_.push_back(markable);
}

void QuickMarkMux::unregisterAllMarkables() {
  for (auto Markable : registeredMarkables_) disconnect(Markable, 0, this, 0);

  registeredMarkables_.clear();
}
