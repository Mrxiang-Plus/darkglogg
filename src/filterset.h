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

#ifndef FILTERSET_H
#define FILTERSET_H

#include <configuration.h>
#include <QColor>
#include <QMetaType>
#include <QRegularExpression>

#include <boost/shared_ptr.hpp>

#include "persistable.h"

// Represents a filter, i.e. a regexp and the colors matching text
// should be rendered in.
class Filter {
 public:
  // Construct an uninitialized Filter (when reading from a config file)
  Filter();
  Filter(const QString& description, const QString& pattern, bool ignoreCase,
         bool ignoreColor, const QString& foreColor, const QString& backColor);

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
  void setBackColor(const QString& backColorName);
  void setDescription(const QString& description);

  // Operators for serialization
  // (must be kept to migrate filters from <=0.8.2)
  friend QDataStream& operator<<(QDataStream& out, const Filter& object);
  friend QDataStream& operator>>(QDataStream& in, Filter& object);

  // Reads/writes the current config in the QSettings object passed
  void saveToStorage(QSettings& settings) const;
  void retrieveFromStorage(QSettings& settings);

 private:
  QRegularExpression regexp_;
  bool ignoreColor_;
  QString foreColorName_;
  QString backColorName_;
  QString description_;
  bool enabled_;
};

// Represents an ordered set of filters to be applied to each line displayed.
class FilterSet : public Persistable {
 public:
  // Construct an empty filter set
  FilterSet();

  // Returns weither the passed line match a filter of the set,
  // if so, it returns the fore/back colors the line should use.
  // Ownership of the colors is transfered to the caller.
  bool matchLine(const QString& line, QColor* foreColor,
                 QColor* backColor) const;

  // Reads/writes the current config in the QSettings object passed
  virtual void saveToStorage(QSettings& settings) const;
  virtual void retrieveFromStorage(QSettings& settings);

  // Should be private really, but I don't know how to have
  // it recognised by QVariant then.
  typedef QList<Filter> FilterList;

  // Operators for serialization
  // (must be kept to migrate filters from <=0.8.2)
  friend QDataStream& operator<<(QDataStream& out, const FilterSet& object);
  friend QDataStream& operator>>(QDataStream& in, FilterSet& object);

 private:
  static const int FILTERSET_VERSION;

  FilterList filterList;

  // To simplify this class interface, FilterDialog can access our
  // internal structure directly.
  friend class FiltersDialog;
};

Q_DECLARE_METATYPE(Filter)
Q_DECLARE_METATYPE(FilterSet)
Q_DECLARE_METATYPE(FilterSet::FilterList)

#endif
