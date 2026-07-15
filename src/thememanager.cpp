/*
 * Copyright (C) 2024 darkglogg contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "thememanager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QStandardPaths>

#include "log.h"

ThemeManager::ThemeManager() {}

ThemeManager& ThemeManager::instance() {
    static ThemeManager tm;
    return tm;
}

bool ThemeManager::loadTheme(const QString& path) {
    LOG(logERROR) << "ThemeManager::loadTheme path=" << path.toStdString();
    if (path.isEmpty()) {
        LOG(logWARNING) << "ThemeManager: empty path";
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG(logWARNING) << "ThemeManager: cannot open " << path.toStdString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    LOG(logERROR) << "ThemeManager: read " << data.size() << " bytes";

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        LOG(logWARNING) << "ThemeManager: JSON parse error: " << error.errorString().toStdString();
        return false;
    }

    if (!doc.isObject()) {
        LOG(logWARNING) << "ThemeManager: root is not an object";
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("colors") || !root["colors"].isObject()) {
        LOG(logWARNING) << "ThemeManager: missing 'colors' object";
        return false;
    }

    themeData_ = root;
    currentPath_ = path;

    LOG(logINFO) << "ThemeManager: loaded theme '" << currentThemeName().toStdString()
                 << "' from " << path.toStdString();
    return true;
}

QColor ThemeManager::color(const QString& key, const QColor& fallback) const {
    if (!themeData_.contains("colors"))
        return fallback;

    QJsonObject colors = themeData_["colors"].toObject();
    if (!colors.contains(key))
        return fallback;

    QString hex = colors[key].toString();
    QColor c(hex);
    return c.isValid() ? c : fallback;
}

QStringList ThemeManager::builtinThemes() const {
    QStringList themes;
    QSet<QString> seenNames;

    // Built-in themes from resources (priority)
    QDirIterator it(":/themes", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (path.endsWith(".json")) {
            QString name = themeName(path);
            if (!seenNames.contains(name)) {
                themes << path;
                seenNames.insert(name);
            }
        }
    }

    // User themes from ~/.glogg/themes/ (skip duplicates by name)
    QString userThemeDir = QDir::homePath() + "/.glogg/themes";
    QDir dir(userThemeDir);
    if (dir.exists()) {
        QDirIterator uit(userThemeDir, QStringList() << "*.json", QDir::Files);
        while (uit.hasNext()) {
            QString path = uit.next();
            QString name = themeName(path);
            if (!seenNames.contains(name)) {
                themes << path;
                seenNames.insert(name);
            }
        }
    }

    themes.sort();
    return themes;
}

QString ThemeManager::themeName(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QFileInfo(path).baseName();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject() && doc.object().contains("name"))
        return doc.object()["name"].toString();

    return QFileInfo(path).baseName();
}

QString ThemeManager::currentThemeName() const {
    if (themeData_.contains("name"))
        return themeData_["name"].toString();
    return QFileInfo(currentPath_).baseName();
}

QString ThemeManager::currentThemePath() const {
    return currentPath_;
}
