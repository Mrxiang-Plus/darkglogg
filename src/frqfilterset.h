/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
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

#ifndef FRQFILTERSET_H
#define FRQFILTERSET_H

#include <QColor>
#include <QMetaType>
#include <QRegularExpression>

#include "persistable.h"

// Represents a FrqFilter, i.e. a regexp and the colors matching text
// should be rendered in.
class FrqFilter {
 public:
  // Construct an uninitialized FrqFilter (when reading from a config file)
  FrqFilter();
  FrqFilter(const QString& description, const QString& pattern, bool ignoreCase,
            bool ignoreColor, const QString& foreColor,
            const QString& backColor);

  bool hasMatch(const QString& string) const;

  // Accessor functions
  QString pattern() const;
  void setPattern(const QString& pattern);
  bool ignoreCase() const;
  bool ignoreColor() const;
  void setIgnoreCase(bool ignoreCase);
  void setIgnoreColor(bool ignoreColor);
  const QString& foreColorName() const;
  const QColor foreColor() const;
  void setForeColor(const QString& foreColorName);
  const QString& backColorName() const;
  const QString& description() const;
  const QColor backColor() const;
  void ContrastColor(QColor* color, QColor* foreColor) const;
  void setBackColor(const QString& backColorName);
  void setDescription(const QString& description);
  void setEnabled(const bool enabled);
  bool getEnabled() const;

  // Operators for serialization
  // (must be kept to migrate FrqFilters from <=0.8.2)
  friend QDataStream& operator<<(QDataStream& out, const FrqFilter& object);
  friend QDataStream& operator>>(QDataStream& in, FrqFilter& object);

  // Reads/writes the current config in the QSettings object passed
  void saveToStorage(QSettings& settings) const;
  void retrieveFromStorage(QSettings& settings);

 private:
  QRegularExpression regexp_;
  QRegularExpression regexpreal_;
  QString foreColorName_;
  QString backColorName_;
  QString description_;
  bool ignoreColor_;
  bool enabled_;
};

// Represents an ordered set of FrqFilters to be applied to each line displayed.
class FrqFilterSet : public Persistable {
 public:
  // Construct an empty FrqFilter set
  FrqFilterSet();

  int getSize() const;
  QString getPinedFilters(const int index) const;
  QString getPinedDescription(const int index) const;
  void getPinedFiltersColor(const int index, QColor* foreColor,
                            QColor* backColor) const;
  // Returns weither the passed line match a FrqFilter of the set,
  // if so, it returns the fore/back colors the line should use.
  // Ownership of the colors is transfered to the caller.
  bool matchLine(const QString& line, QColor* foreColor,
                 QColor* backColor) const;

  bool matchFirstLine(const QString& line, QColor* foreColor,
                      QColor* backColor) const;

  // Reads/writes the current config in the QSettings object passed
  virtual void saveToStorage(QSettings& settings) const;
  virtual void retrieveFromStorage(QSettings& settings);

  // Should be private really, but I don't know how to have
  // it recognised by QVariant then.
  typedef QList<FrqFilter> FrqFilterList;

  // Operators for serialization
  // (must be kept to migrate FrqFilters from <=0.8.2)
  friend QDataStream& operator<<(QDataStream& out, const FrqFilterSet& object);
  friend QDataStream& operator>>(QDataStream& in, FrqFilterSet& object);

  FrqFilterList frqFilterList;

 private:
  static const int FRQFILTERSET_VERSION;

  // To simplify this class interface, FrqFilterDialog can access our
  // internal structure directly.
  friend class FrqFiltersDialog;
};

Q_DECLARE_METATYPE(FrqFilter)
Q_DECLARE_METATYPE(FrqFilterSet)
Q_DECLARE_METATYPE(FrqFilterSet::FrqFilterList)

#endif
