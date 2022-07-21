#include "versionmanager.h"

#include "log.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QProcess>
#include <QFile>
#include <QDir>

const char* VersionManager::CUR_VERSION = "V1.1";
const char* VersionManager::VERSION_URL =
        "https://git.n.xiaomi.com/wanghuiting1/gloggversion/-/raw/master/version.json?inline=false";
const char* VersionManager::VERSION_JSON = "version.json";
const QString VersionManager::WORK_DIR = QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

VersionManager::VersionManager(QObject *parent) : QObject(parent)
{

}

QString VersionManager::getCurVersion()
{
    return CUR_VERSION;
}

void VersionManager::startCheck(bool onlyCheck)
{
    justCheck = onlyCheck;
    QProcess* process = new QProcess();
    QString path = QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();

    process->execute("/bin/bash", QStringList() << WORK_DIR + "download_tool.sh" << VERSION_URL << VERSION_JSON);
    getVersionJson();
}

void VersionManager::getVersionJson()
{
    QFile version(WORK_DIR + VERSION_JSON);
    version.open(QIODevice::ReadOnly);
    QByteArray data = version.readAll();
    version.close();
    if (justCheck)
        onlyCheckVersion(data);
    else
        parse_UpdateVersion(data);

}

void VersionManager::onlyCheckVersion(QByteArray byteData)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(QString::fromLocal8Bit(byteData).toUtf8(), &err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "json error" << err_rpt.errorString();
        return;
    }
    if(root_Doc.isObject())
    {
        QJsonObject  root_Obj = root_Doc.object();   //创建JSON对象，不是字符串
        QJsonObject GloggValue = root_Obj.value("Glogg_Update").toObject();
        QString Verison = GloggValue.value("LatestVerison").toString();
        if(Verison > CUR_VERSION)
        {
            emit existLatestVersion(true);
        }
    }
}

void VersionManager::parse_UpdateVersion(QByteArray byteData)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(QString::fromLocal8Bit(byteData).toUtf8(), &err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "json error" << err_rpt.errorString();
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                                              "Update failed",
                                              tr("Server address error!"
                                                 "<p>Please manually confirm whether you have permission to access the following address."
                                                 "<p>https://cnbj1-fds.api.xiaomi.net/camera-devtest/07-Tools/glogg/version.json"));
        msgBox->show();
        return;
    }
    if(root_Doc.isObject())
    {
        QJsonObject  root_Obj = root_Doc.object();   //创建JSON对象，不是字符串
        QJsonObject GloggValue = root_Obj.value("Glogg_Update").toObject();
        QString Verison = GloggValue.value("LatestVerison").toString();
        QString UpdateUrl = GloggValue.value("UpdateUrl").toString();
        QString UpdateTime = GloggValue.value("UpdateTime").toString();
        QString ReleaseNote = GloggValue.value("ReleaseNote").toString();
        if(Verison > CUR_VERSION)
        {
            QString updateStr =  "Detect new version!\nLatest version:" + Verison + "\n" + "Update time:" + UpdateTime + "\n" + ReleaseNote;
            QMessageBox *msgBoxUpdate = new QMessageBox(QMessageBox::Information, "Check Update", updateStr);
            QPushButton *updateBtn = msgBoxUpdate->addButton(tr("Update"), QMessageBox::AcceptRole);
            QPushButton *ignoreBtn = msgBoxUpdate->addButton(tr("Ignore"), QMessageBox::RejectRole);
            msgBoxUpdate->setDefaultButton(updateBtn);
            msgBoxUpdate->exec();
            if ((QPushButton*)msgBoxUpdate->clickedButton() == updateBtn)    //点击更新
            {
                QProcess* process = new QProcess();
                QString path = QDir::homePath() + QDir::separator() + ".glogg" + QDir::separator();
                process->start("/bin/bash", QStringList() << WORK_DIR + "update_version.sh" << UpdateUrl);
                QMessageBox::information(NULL, "Install", tr("<h4>Download finish. But need user permission.</h4>"
                                                             "<h4>Please run the following command on the terminal.</h4>"
                                                             "<h4>cd ~/.glogg/release && ./install.sh</h4>"));
            }
        }
        else
            QMessageBox::information(NULL, "Check Update", "The current version is the latest!");
    }
}


