/*
 * Copyright (C) 2009, 2010, 2011 Nicolas Bonnefon and other contributors
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

// This file implements classes FrqFilter and FrqFilterSet

#include <configuration.h>
#include <frqfilterset.h>
#include <QDataStream>
#include <QSettings>

#include <boost/shared_ptr.hpp>

#include "log.h"
#include "persistentinfo.h"

const int FrqFilterSet::FRQFILTERSET_VERSION = 1;

QRegularExpression::PatternOptions getFilterPatternOptions(bool ignoreCase) {
  QRegularExpression::PatternOptions options =
      QRegularExpression::UseUnicodePropertiesOption |
      QRegularExpression::OptimizeOnFirstUsageOption;

  if (ignoreCase) {
    options |= QRegularExpression::CaseInsensitiveOption;
  }
  return options;
}

FrqFilter::FrqFilter() {}

FrqFilter::FrqFilter(const QString& description, const QString& pattern,
                     bool ignoreCase, bool ignoreColor,
                     const QString& foreColorName, const QString& backColorName)
    : regexp_(pattern, getFilterPatternOptions(ignoreCase)),
      ignoreColor_(ignoreColor),
      foreColorName_(foreColorName),
      backColorName_(backColorName),
      description_(description),
      enabled_(true) {
  LOG(logDEBUG) << "New FrqFilter, fore: " << foreColorName_.toStdString()
                << " back: " << backColorName_.toStdString();
}

QString FrqFilter::pattern() const { return regexp_.pattern(); }

void FrqFilter::setPattern(const QString& pattern) {
  regexp_.setPattern(pattern);
}

bool FrqFilter::ignoreCase() const {
  return regexp_.patternOptions().testFlag(
      QRegularExpression::CaseInsensitiveOption);
}

void FrqFilter::setIgnoreCase(bool ignoreCase) {
  regexp_.setPatternOptions(getFilterPatternOptions(ignoreCase));
}

bool FrqFilter::ignoreColor() const { return ignoreColor_; }
void FrqFilter::setIgnoreColor(bool ignoreColor) { ignoreColor_ = ignoreColor; }

const QString& FrqFilter::foreColorName() const { return foreColorName_; }

const QColor FrqFilter::foreColor() const {
  if (foreColorName_.toStdString() == "window") {
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    if (config->wasdStyle()) {
      return QColor(33, 33, 33);
    } else {
      return QColor(239, 235, 231);
    }
  } else if (foreColorName_.toStdString() == "text") {
    return QColor(255, 183, 97);
  } else {
    return QColor(foreColorName_);
  }
}

void FrqFilter::ContrastColor(QColor* color, QColor* foreColor) const {
  int d = 0;

  // Counting the perceptive luminance - human eye favors green color...
  double luminance =
      (0.299 * color->red() + 0.587 * color->green() + 0.114 * color->blue()) /
      255;

  if (luminance > 0.5)
    d = 0;  // bright colors - black font
  else
    d = 255;  // dark colors - white font

  foreColor->setRgb(d, d, d);
}

const QColor FrqFilter::backColor() const {
  if (backColorName_.toStdString() == "window") {
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>("settings");
    if (config->wasdStyle()) {
      return QColor(33, 33, 33);
    } else {
      return QColor(239, 235, 231);
    }
  } else if (backColorName_.toStdString() == "text") {
    return QColor(255, 203, 107);
  } else {
    return QColor(backColorName_);
  }
}

void FrqFilter::setForeColor(const QString& foreColorName) {
  foreColorName_ = foreColorName;
}

const QString& FrqFilter::backColorName() const { return backColorName_; }

void FrqFilter::setBackColor(const QString& backColorName) {
  backColorName_ = backColorName;
}

const QString& FrqFilter::description() const { return description_; }

void FrqFilter::setDescription(const QString& description) {
  description_ = description;
}

void FrqFilter::setEnabled(const bool enabled) { enabled_ = enabled; }

bool FrqFilter::getEnabled() const { return enabled_; }

bool FrqFilter::hasMatch(const QString& string) const {
  return regexp_.match(string).hasMatch();
}

//
// Operators for serialization
//

QDataStream& operator<<(QDataStream& out, const FrqFilter& object) {
  LOG(logDEBUG) << "<<operator from FrqFilter";
  out << object.regexp_;
  out << object.ignoreColor_;
  out << object.foreColorName_;
  out << object.backColorName_;
  out << object.description_;
  out << object.enabled_;

  return out;
}

QDataStream& operator>>(QDataStream& in, FrqFilter& object) {
  LOG(logDEBUG) << ">>operator from FrqFilter";
  in >> object.regexp_;
  in >> object.ignoreColor_;
  in >> object.foreColorName_;
  in >> object.backColorName_;
  in >> object.description_;
  in >> object.enabled_;

  return in;
}

// Default constructor
FrqFilterSet::FrqFilterSet() {
  qRegisterMetaTypeStreamOperators<FrqFilter>("FrqFilter");
  qRegisterMetaTypeStreamOperators<FrqFilterSet>("FrqFilterSet");
  qRegisterMetaTypeStreamOperators<FrqFilterSet::FrqFilterList>(
      "FrqFilterSet::FrqFilterList");
}

QString FrqFilterSet::getPinedFilters(const int index) const {
  return frqFilterList.at(index).pattern();
}

QString FrqFilterSet::getPinedDescription(const int index) const {
  return frqFilterList.at(index).description();
}

void FrqFilterSet::getPinedFiltersColor(const int index, QColor* foreColor,
                                        QColor* backColor) const {
  FrqFilter frqFilter = frqFilterList.at(index);
  QColor fColor = frqFilter.foreColor();
  QColor bColor = frqFilter.backColor();
  foreColor->setRgb(fColor.red(), fColor.green(), fColor.blue());
  backColor->setRgb(bColor.red(), bColor.green(), bColor.blue());
}

int FrqFilterSet::getSize() const { return frqFilterList.size(); }

bool FrqFilterSet::matchLine(const QString& line, QColor* foreColor,
                             QColor* backColor) const {
  if (frqFilterList.size() > 0) {
    for (QList<FrqFilter>::const_iterator i = frqFilterList.constBegin();
         i != frqFilterList.constEnd(); i++) {
      if (i->hasMatch(line) && i->getEnabled() && !i->ignoreColor()) {
        QColor fColor = i->foreColor();
        QColor bColor = i->backColor();
        foreColor->setRgb(fColor.red(), fColor.green(), fColor.blue());
        backColor->setRgb(bColor.red(), bColor.green(), bColor.blue());
        return true;
      }
    }
  }
  return false;
}

bool FrqFilterSet::matchFirstLine(const QString& line, QColor* foreColor,
                                  QColor* backColor) const {
  if (frqFilterList.size() > 0) {
    QList<FrqFilter>::const_iterator i = frqFilterList.constBegin();
    if (i->hasMatch(line) && i->getEnabled()) {
      QColor fColor = i->foreColor();
      QColor bColor = i->backColor();
      foreColor->setRgb(fColor.red(), fColor.green(), fColor.blue());
      backColor->setRgb(bColor.red(), bColor.green(), bColor.blue());
      return true;
    }
  }

  return false;
}

//
// Operators for serialization
//

QDataStream& operator<<(QDataStream& out, const FrqFilterSet& object) {
  LOG(logDEBUG) << "<<operator from FrqFilterSet";
  out << object.frqFilterList;

  return out;
}

QDataStream& operator>>(QDataStream& in, FrqFilterSet& object) {
  LOG(logDEBUG) << ">>operator from FrqFilterSet";
  in >> object.frqFilterList;

  return in;
}

//
// Persistable virtual functions implementation
//

void FrqFilter::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "FrqFilter::saveToStorage";

  settings.setValue("description", description_);
  settings.setValue("regexp", regexp_.pattern());
  settings.setValue("ignore_case",
                    regexp_.patternOptions().testFlag(
                        QRegularExpression::CaseInsensitiveOption));
  settings.setValue("ignore_color", ignoreColor_);
  settings.setValue("fore_colour", foreColorName_);
  settings.setValue("back_colour", backColorName_);
  settings.setValue("enabled", enabled_);
}

void FrqFilter::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "FrqFilter::retrieveFromStorage";

  description_ = settings.value("description").toString();
  regexp_ = QRegularExpression(
      settings.value("regexp").toString(),
      getFilterPatternOptions(settings.value("ignore_case", false).toBool()));

  ignoreColor_ = settings.value("ignore_color").toBool();
  foreColorName_ = settings.value("fore_colour").toString();
  backColorName_ = settings.value("back_colour").toString();
  enabled_ = settings.value("enabled").toBool();
}

void FrqFilterSet::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "FrqFilterSet::saveToStorage";

  settings.beginGroup("FrqFilterSet");
  // Remove everything in case the array is shorter than the previous one
  settings.remove("");
  settings.setValue("version", FRQFILTERSET_VERSION);
  settings.beginWriteArray("FrqFilters");
  for (int i = 0; i < frqFilterList.size(); ++i) {
    settings.setArrayIndex(i);
    frqFilterList[i].saveToStorage(settings);
  }
  settings.endArray();
  settings.endGroup();
}

void FrqFilterSet::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "FrqFilterSet::retrieveFromStorage";

  frqFilterList.clear();

  if (settings.contains("FrqFilterSet/version")) {
    settings.beginGroup("FrqFilterSet");
    if (settings.value("version") == FRQFILTERSET_VERSION) {
      int size = settings.beginReadArray("FrqFilters");
      for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        FrqFilter FrqFilter;
        FrqFilter.retrieveFromStorage(settings);
        frqFilterList.append(FrqFilter);
      }
      settings.endArray();
    } else {
      LOG(logERROR) << "Unknown version of FrqFilterSet, ignoring it...";
    }
    settings.endGroup();
  } else {
    LOG(logWARNING) << "Trying to import legacy (<=0.8.2) FrqFilters...";
    FrqFilterSet tmp_FrqFilter_set =
        settings.value("FrqFilterSet").value<FrqFilterSet>();
    *this = tmp_FrqFilter_set;
    LOG(logWARNING) << "...imported FrqFilterSet: " << frqFilterList.count()
                    << " elements";
    // Remove the old key once migration is done
    settings.remove("FrqFilterSet");
    // And replace it with the new one
    saveToStorage(settings);
    settings.sync();
  }
}
