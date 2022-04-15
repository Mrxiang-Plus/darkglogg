#include "sharedfilterset.h"
#include <QDataStream>
#include <QSettings>

#include "log.h"
#include "persistentinfo.h"

const int SharedFilterSet::SHAREDFILTERSET_VERSION = 1;

SharedFilterSet::SharedFilterSet()
{

    qRegisterMetaTypeStreamOperators<SharedFilter>("SharedFilter");
    qRegisterMetaTypeStreamOperators<SharedFilterSet>("SharedFilterSet");
    qRegisterMetaTypeStreamOperators<SharedFilterSet::SharedFilterList>(
        "SharedFilterSet::SharedFilterList");
}

QDataStream& operator<<(QDataStream& out, const SharedFilterSet& object) {
  LOG(logDEBUG) << "<<operator from SharedFilterSet";
  out << object.sharedFilterList;

  return out;
}

QDataStream& operator>>(QDataStream& in, SharedFilterSet& object) {
  LOG(logDEBUG) << ">>operator from SharedFilterSet";
  in >> object.sharedFilterList;

  return in;
}

SharedFilter::SharedFilter() {}

SharedFilter::SharedFilter(const QString& key, const QString& pattern, const QString& comment)
    : regexp_(key),
      pattern_(pattern),
      comment_(comment)
{
//  LOG(logDEBUG) << "New Filter, key: " << regexp_.toStdString()
//                << " pattern: " << pattern_.toStdString()
//                << " comment: " << comment_.toStdString();
}

//const QString& SharedFilter::key() const { return regexp_; }

//void SharedFilter::setKey(const QString& key)
//{
//  regexp_ = key;
//}

const QString& SharedFilter::pattern() const { return pattern_; }

void SharedFilter::setPattern(const QString& pattern)
{
    pattern_ = pattern;
}


const QString& SharedFilter::comment() const { return comment_; }

void SharedFilter::setComment(const QString& comment)
{
  comment_ = comment;
}

bool SharedFilter::hasMatch(const QString& string) const {
  return regexp_.match(string).hasMatch();
}

//
// Operators for serialization
//

QDataStream& operator<<(QDataStream& out, const SharedFilter& object) {
  LOG(logDEBUG) << "<<operator from SharedFilter";
  out << object.regexp_;
  out << object.pattern_;
  out << object.comment_;

  return out;
}

QDataStream& operator>>(QDataStream& in, SharedFilter& object) {
  LOG(logDEBUG) << ">>operator from SharedFilter";
  in >> object.regexp_;
  in >> object.pattern_;
  in >> object.comment_;

  return in;
}

//
// Persistable virtual functions implementation
//

void SharedFilter::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "SharedFilter::saveToStorage";

  settings.setValue("key", regexp_);
  settings.setValue("pattern", pattern_);
  settings.setValue("comment", comment_);
}

void SharedFilter::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "SharedFilter::retrieveFromStorage";

//  regexp_ = settings.value("key").toString();
  pattern_ = settings.value("pattern").toString();
  comment_ = settings.value("comment").toString();
}

//============================================================//












void SharedFilterSet::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "SharedFilterSet::saveToStorage";

  settings.beginGroup("SharedFilterSet");
  // Remove everything in case the array is shorter than the previous one
  settings.remove("");
  settings.setValue("version", SHAREDFILTERSET_VERSION);
  settings.beginWriteArray("sharedfilter");
  for (int i = 0; i < sharedFilterList.size(); ++i) {
    settings.setArrayIndex(i);
    sharedFilterList[i].saveToStorage(settings);
  }
  settings.endArray();
  settings.endGroup();
}

void SharedFilterSet::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "SharedFilterSet::retrieveFromStorage";

  sharedFilterList.clear();

  if (settings.contains("SharedFilterSet/version")) {
    settings.beginGroup("SharedFilterSet");
    if (settings.value("version") == SHAREDFILTERSET_VERSION) {
      int size = settings.beginReadArray("sharedfilter");
      for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        SharedFilter sharedFilter;
        sharedFilter.retrieveFromStorage(settings);
        sharedFilterList.append(sharedFilter);
      }
      settings.endArray();
    } else {
      LOG(logERROR) << "Unknown version of SharedFilterSet, ignoring it...";
    }
    settings.endGroup();
  } else {
    LOG(logWARNING) << "Trying to import legacy (<=0.8.2) filters...";
    SharedFilterSet tmp_filter_set = settings.value("SharedFilterSet").value<SharedFilterSet>();
    *this = tmp_filter_set;
    LOG(logWARNING) << "...imported SharedFilterSet: " << sharedFilterList.count()
                    << " elements";
    // Remove the old key once migration is done
    settings.remove("SharedFilterSet");
    // And replace it with the new one
    saveToStorage(settings);
    settings.sync();
  }
}
