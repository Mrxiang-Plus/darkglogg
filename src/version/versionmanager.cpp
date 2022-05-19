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

const char* VersionManager::CUR_VERSION = "V1.0";
const char* VersionManager::VERSION_URL =
        "https://cnbj1-fds.api.xiaomi.net/camera-devtest/07-Tools/glogg/version.json";

VersionManager::VersionManager(QObject *parent) : QObject(parent)
{
    manager_ = new QNetworkAccessManager(this);
    connect(manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

QString VersionManager::getCurVersion()
{
    return CUR_VERSION;
}

void VersionManager::startCheck(bool onlyCheck)
{
    QNetworkRequest request;
    justCheck = onlyCheck;
    request.setUrl(QUrl(VERSION_URL));
    manager_->get(request);
}

void VersionManager::replyFinished(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode != 200)
    {
        LOG(logDEBUG) << "VersionManager: status code of reply is 200";
        return;
    }
    QString str = reply->readAll();
    if (justCheck)
        onlyCheckVersion(str);
    else
        parse_UpdateVersion(str);

    reply->deleteLater();
}

void VersionManager::onlyCheckVersion(QString str)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(str.toUtf8(),&err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
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

void VersionManager::parse_UpdateVersion(QString str)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(str.toUtf8(),&err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                                              "Update failed",
                                              tr("Server address error!"
                                                 "<p>Please manually confirm whether you have permission to access the following address."
                                                 "<p>https://cnbj1-fds.api.xiaomi.net/camera-devtest/07-Tools/glogg/version.json"));
        msgBox->show();
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
                QDesktopServices::openUrl(QUrl(UpdateUrl));
            }
//            else if ((QPushButton*)msgBoxUpdate->clickedButton() == ignoreBtn)
//            {
//                emit existLatestVersion(false);
//            }
        }
        else
            QMessageBox::information(NULL, "Check Update", "The current version is the latest!");
    }
}


