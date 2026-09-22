#include "sharedfilterset.h"
#include <QDataStream>
#include <QSettings>

#include "log.h"
#include "persistentinfo.h"

const int SharedFilterSet::SHAREDFILTERSET_VERSION = 1;

QRegularExpression::PatternOptions getPatternOptions() {
  QRegularExpression::PatternOptions options =
      QRegularExpression::UseUnicodePropertiesOption |
      QRegularExpression::CaseInsensitiveOption;
  return options;
}

SharedFilterSet::SharedFilterSet()
{

    qRegisterMetaType<SharedFilter>("SharedFilter");
    qRegisterMetaType<SharedFilterSet>("SharedFilterSet");
    qRegisterMetaType<SharedFilterSet::SharedFilterList>(
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

SharedFilter::SharedFilter(const QString& tab, const QString& key, const QString& pattern, const QString& comment)
    : tab_(tab),
      regexp_(key, getPatternOptions()),
      pattern_(pattern),
      comment_(comment)
{
  LOG(logDEBUG) << "New Filter, tab: " << tab_.toStdString()
                << "key: " << regexp_.pattern().toStdString()
                << " pattern: " << pattern_.toStdString()
                << " comment: " << comment_.toStdString();
}

const QString& SharedFilter::tab() const { return tab_; }

void SharedFilter::setTab(const QString& tab)
{
    tab_ = tab;
}

QString SharedFilter::key() const { return regexp_.pattern(); }

void SharedFilter::setKey(const QString& key)
{
  regexp_.setPattern(key);
}

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

const QString& SharedFilter::filterItem() const
{
    return filterItem_;
}

void SharedFilter::setFilterItem()
{
    filterItem_ = tab_;
    filterItem_.append("#");
    filterItem_.append(regexp_.pattern());
    filterItem_.append(">");
    filterItem_.append(pattern_);
    if (!comment_.isNull())
    {
        filterItem_.append(">");
        filterItem_.append(comment_);
    }
}

void SharedFilter::retrieveFromFilterItem()
{
    QStringList tempList = filterItem_.split("#");
    tab_ = tempList.at(0);
    if (tempList.size() > 1)
    {
        QString tempStr = tempList.at(1);
        tempList = tempStr.split(">");
        regexp_.setPattern(tempList.at(0));
        pattern_ = tempList.at(1);
        if (tempList.size() > 2) {
            comment_ = tempList.at(2);
        }
    }
}

bool SharedFilter::hasMatch(const QString& string) const {
  return QString::compare(regexp_.pattern(), string) == 0;
}

//
// Operators for serialization
//

QDataStream& operator<<(QDataStream& out, const SharedFilter& object) {
  LOG(logDEBUG) << "<<operator from SharedFilter";
//  out << object.tab_;
//  out << object.regexp_;
//  out << object.pattern_;
//  out << object.comment_;
  out << object.filterItem_;

  return out;
}

QDataStream& operator>>(QDataStream& in, SharedFilter& object) {
  LOG(logDEBUG) << ">>operator from SharedFilter";
//  in >> object.tab_;
//  in >> object.regexp_;
//  in >> object.pattern_;
//  in >> object.comment_;
  in >> object.filterItem_;


  return in;
}

//
// Persistable virtual functions implementation
//

void SharedFilter::saveToStorage(QSettings& settings) const {
  LOG(logDEBUG) << "SharedFilter::saveToStorage";
  settings.setValue("filterItem", filterItem_);

//  settings.setValue("tab", tab_);
//  settings.setValue("key", regexp_.pattern());
//  settings.setValue("pattern", pattern_);
//  settings.setValue("comment", comment_);
}

void SharedFilter::retrieveFromStorage(QSettings& settings) {
  LOG(logDEBUG) << "SharedFilter::retrieveFromStorage";

  filterItem_ = settings.value("filterItem").toString();
  QStringList tempList = filterItem_.split("#");
  tab_ = tempList.at(0);
  if (tempList.size() > 1)
  {
      QString tempStr = tempList.at(1);
      tempList = tempStr.split(">");
      regexp_.setPattern(tempList.at(0));
      pattern_ = tempList.at(1);
      if (tempList.size() > 2) {
          comment_ = tempList.at(2);
      }
  }


//  tab_ = settings.value("tab").toString();
//  regexp_ = QRegularExpression(
//              settings.value("key").toString(),
//              getPatternOptions());
//  pattern_ = settings.value("pattern").toString();
//  comment_ = settings.value("comment").toString();
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
