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

#ifndef QUICKMARKMUX_H
#define QUICKMARKMUX_H

#include <memory>
#include <vector>

#include <QObject>
#include <QString>

#include "quickfindpattern.h"
#include "quickfindwidget.h"

// Interface representing a widget Markable in both direction.
class MarkableWidgetInterface {
 public:
  virtual void markForward() = 0;
  virtual void markBackward() = 0;

  virtual void incrementallyMarkForward() = 0;
  virtual void incrementallyMarkBackward() = 0;
  virtual void incrementalMarkStop() = 0;
  virtual void incrementalMarkAbort() = 0;
};

// Interface representing the selector. It will be called and asked
// who the Mark have to be forwarded to.
class QuickMarkMuxSelectorInterface {
 public:
  // Return the Markable widget to use.
  MarkableWidgetInterface* getActiveMarkable() const {
    return doGetActiveMarkable();
  }
  // Return the list of all possible Markables, this
  // is done on registration in order to establish
  // listeners on all Markables.
  std::vector<QObject*> getAllMarkables() const { return doGetAllMarkables(); }

 protected:
  virtual MarkableWidgetInterface* doGetActiveMarkable() const = 0;
  virtual std::vector<QObject*> doGetAllMarkables() const = 0;
};

class QFNotification;

// Represents a multiplexer (unique application wise) dispatching the
// Quick Find Mark from the UI to the relevant view.
// It is also its responsability to determine if an incremental Mark
// must be performed and to react accordingly.
class QuickMarkMux : public QObject {
  Q_OBJECT

 public:
  // Construct the multiplexer, taking a reference to the pattern
  QuickMarkMux(std::shared_ptr<QuickFindPattern> pattern);

  // Register a new selector, which will be called and asked
  // who the Mark have to be forwarded to.
  // The selector is called immediately when registering to get the list of
  // Markables.
  // The previous selector and its associated views are automatically
  // deregistered.
  // A null selector is accepted, in this case QFM functionalities are
  // disabled until a valid selector is registered.
  void registerSelector(const QuickMarkMuxSelectorInterface* selector);

  // Set the direction that will be used by the Mark when Marking
  // forward.
  void setDirection(QFDirection direction);

 signals:
  void patternChanged(const QString&);
  void notify(const QFNotification&);
  void clearNotification();

 public slots:
  // Signal the current pattern must be altered (will start an incremental
  // Mark if the options are configured in such a way).
  void setNewPattern(const QString& new_pattern, bool ignore_case);

  // Signal the current pattern must be altered and is confirmed
  // (will stop an incremental Mark if needed)
  void confirmPattern(const QString& new_pattern, bool ignore_case,
                      QFDirection direction);
  void confirmPatternWithoutSearch(const QString& new_pattern,
                                   bool ignore_case);

  // Signal the user cancelled the Mark
  // (used for incremental only)
  void cancelMark();

  // Starts a Mark in the specified direction
  void markNext();
  void markPrevious();

  // Idem but ignore the direction and always mark in the
  // specified direction
  void markForward();
  void markBackward();

 private slots:
  void changeQuickMark(const QString& new_pattern, QFDirection new_direction);
  void notifyPatternChanged(QList<int> removedList);

 private:
  void markNext(QFDirection);
  void markPrevious(QFDirection);
  const QuickMarkMuxSelectorInterface* selector_;

  // The (application wide) quick find pattern
  std::shared_ptr<QuickFindPattern> pattern_;

  QFDirection currentDirection_;

  std::vector<QObject*> registeredMarkables_;

  MarkableWidgetInterface* getMarkableWidget() const;
  void registerMarkable(QObject* narkable);
  void unregisterAllMarkables();
};

#endif
