#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QProcess>

class VersionManager : public QObject
{
    Q_OBJECT
public:
    explicit VersionManager(QObject *parent = nullptr);   
    // Starts an asynchronous check for a newer version if it is needed.
    // A newVersionFound signal is sent if one is found.
    // In case of error or if no new version is found, no signal is emitted.
    void startCheck(bool onlyCheck);
    QString getCurVersion();

signals:
    // New version "version" is available
    void newVersionFound(const QString& version);
    bool existLatestVersion(bool isExistent);

private slots:
    // Called when reply is finished

private:
    static const char* VERSION_URL;
    static const char* CUR_VERSION;
    static const char* VERSION_JSON;
    static const QString WORK_DIR;

    static const uint64_t CHECK_INTERVAL_S;

    void getVersionJson();
    void parse_UpdateVersion(QByteArray byteData);
    void onlyCheckVersion(QByteArray byteData);

    bool justCheck;

    QNetworkAccessManager *manager_;
};

#endif // VERSIONMANAGER_H
