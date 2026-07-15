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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QStringList>

class ThemeManager {
public:
    static ThemeManager& instance();

    // Load theme from JSON file (resource path like ":/themes/xxx.json" or filesystem path)
    bool loadTheme(const QString& path);

    // Get color by key with fallback
    QColor color(const QString& key, const QColor& fallback = QColor()) const;

    // Get all built-in theme paths from resources
    QStringList builtinThemes() const;

    // Get display name of a theme file
    QString themeName(const QString& path) const;

    // Current theme name
    QString currentThemeName() const;

    // Current theme path
    QString currentThemePath() const;

private:
    ThemeManager();
    QJsonObject themeData_;
    QString currentPath_;
};

#endif // THEMEMANAGER_H
