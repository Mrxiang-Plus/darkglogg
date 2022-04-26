#ifndef SHAREDFILTERSET_H
#define SHAREDFILTERSET_H

#include <configuration.h>
#include <QColor>
#include <QMetaType>
#include <QRegularExpression>

#include <boost/shared_ptr.hpp>

#include "persistable.h"


class SharedFilter {
 public:
  // Construct an uninitialized Filter (when reading from a config file)
  SharedFilter();
  SharedFilter(const QString& tab, const QString& key, const QString& pattern, const QString& comment);

  bool hasMatch(const QString& string) const;

  // Accessor functions
  const QString& tab() const;
  void setTab(const QString& tab);
  QString key() const;
  void setKey(const QString& key);
  const QString& pattern() const;
  void setPattern(const QString& pattern);
  const QString& comment() const;
  void setComment(const QString& comment);

  // Operators for serialization
  // (must be kept to migrate filters from <=0.8.2)
  friend QDataStream& operator<<(QDataStream& out, const SharedFilter& object);
  friend QDataStream& operator>>(QDataStream& in, SharedFilter& object);

  // Reads/writes the current config in the QSettings object passed
  void saveToStorage(QSettings& settings) const;
  void retrieveFromStorage(QSettings& settings);

 private:
  QString tab_;
  QRegularExpression  regexp_;
  QString pattern_;
  QString comment_;

};

class SharedFilterSet : public Persistable
{
public:
    SharedFilterSet();

//    bool matchLine(const QString& line, QColor* foreColor,
//                   QColor* backColor) const;

    // Reads/writes the current config in the QSettings object passed
    virtual void saveToStorage(QSettings& settings) const;
    virtual void retrieveFromStorage(QSettings& settings);

    // Should be private really, but I don't know how to have
    // it recognised by QVariant then.
    typedef QList<SharedFilter> SharedFilterList;
//    typedef QVector<QVector<SharedFilter>> SharedFilterArray;

    // Operators for serialization
    // (must be kept to migrate filters from <=0.8.2)
    friend QDataStream& operator<<(QDataStream& out, const SharedFilterSet& object);
    friend QDataStream& operator>>(QDataStream& in, SharedFilterSet& object);

   private:
    static const int SHAREDFILTERSET_VERSION;

    SharedFilterList sharedFilterList;
//    SharedFilterArray sharedFilterArray;


    // To simplify this class interface, FilterDialog can access our
    // internal structure directly.
    friend class SharedFilterDialog;
};

  Q_DECLARE_METATYPE(SharedFilter)
  Q_DECLARE_METATYPE(SharedFilterSet)
  Q_DECLARE_METATYPE(SharedFilterSet::SharedFilterList)

#endif // SHAREDFILTERSET_H


