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

// This file implements class SavedSearch

#include <QDataStream>
#include <QSettings>

#include "log.h"
#include "savedpatterns.h"

const int SavedPatterns::SAVEDPATTERNS_VERSION = 1;
const int SavedPatterns::maxNumberOfRecentPatterns = 50000;

SavedPatterns::SavedPatterns() : savedPatterns_() {
  qRegisterMetaType<SavedPatterns>("SavedPatterns");
}

void SavedPatterns::addRecent(const QString& text) {
  // We're not interested in blank lines
  if (text.isEmpty()) return;

  // Remove any copy of the about to be added text
  savedPatterns_.removeAll(text);

  // Add at the front
  savedPatterns_.push_back(text);

  // Trim the list if it's too long
  while (savedPatterns_.size() > maxNumberOfRecentPatterns)
    savedPatterns_.pop_front();
}

QStringList SavedPatterns::recentPatterns() const { return savedPatterns_; }

//
// Operators for serialization
//

QDataStream& operator<<(QDataStream& out, const SavedPatterns& object) {
  LOG(logDEBUG) << "<<operator from SavedPatterns";

  out << object.savedPatterns_;

  return out;
}

QDataStream& operator>>(QDataStream& in, SavedPatterns& object) {
  LOG(logDEBUG) << ">>operator from SavedPatterns";

  in >> object.savedPatterns_;

  return in;
}

//
// Persistable virtual functions implementation
//

void SavedPatterns::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "SavedPatterns::saveToStorage";

  settings.beginGroup("SavedPatterns");
  // Remove everything in case the array is shorter than the previous one
  settings.remove("");
  settings.setValue("version", SAVEDPATTERNS_VERSION);
  settings.beginWriteArray("searchPattern");
  for (int i = 0; i < savedPatterns_.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("string", savedPatterns_.at(i));
  }
  settings.endArray();
  settings.endGroup();
}

void SavedPatterns::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "SavedPatterns::retrieveFromStorage";

  savedPatterns_.clear();

  if (settings.contains("SavedPatterns/version")) {
    // Unserialise the "new style" stored history
    settings.beginGroup("SavedPatterns");
    if (settings.value("version") == SAVEDPATTERNS_VERSION) {
      int size = settings.beginReadArray("searchPattern");
      for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString search = settings.value("string").toString();
        savedPatterns_.append(search);
      }
      settings.endArray();
    } else {
      LOG(logERROR) << "Unknown version of FilterSet, ignoring it...";
    }
    settings.endGroup();
  } else {
    LOG(logWARNING) << "Trying to import legacy (<=0.8.2) saved Patterns...";
    SavedPatterns tmp_saved_Patterns =
        settings.value("savedPatterns").value<SavedPatterns>();
    *this = tmp_saved_Patterns;
    LOG(logWARNING) << "...imported Patterns: " << savedPatterns_.count()
                    << " elements";
    // Remove the old key once migration is done
    settings.remove("savedPatterns");
    // And replace it with the new one
    saveToStorage(settings);
    settings.sync();
  }
}
