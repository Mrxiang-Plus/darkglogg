/*
 * Copyright (C) 2014 Nicolas Bonnefon and other contributors
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

#ifndef TABBEDCRAWLERWIDGET_H
#define TABBEDCRAWLERWIDGET_H

#include <QTabBar>
#include <QTabWidget>

#include "loadingstatus.h"

// This class represents glogg's main widget, a tabbed
// group of CrawlerWidgets.
// This is a very slightly customised QTabWidget, with
// a particular style.
class TabbedCrawlerWidget : public QTabWidget {
  Q_OBJECT
 public:
  TabbedCrawlerWidget(QWidget *);
  virtual ~TabbedCrawlerWidget() {}

  // "Overridden" addTab/removeTab that automatically
  // show/hide the tab bar
  // The tab is created with the 'old data' icon.
  int addTab(QWidget *page, const QString &label, const QString &filePath);
  void removeTab(int index);

  // Set the data status (icon) for the tab number 'index'
  void setTabDataStatus(const QString &filePath, DataStatus status);
  void setTabBarVisibility(bool visible);
  bool getTabBarVisibility();

 protected:
  void keyPressEvent(QKeyEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);

 private:
  const QIcon olddata_icon_;
  const QIcon newdata_icon_;
  const QIcon newfiltered_icon_;

  QTabBar myTabBar_;

 signals:
  void doubleClicked();

 protected:
  QPoint mousePos;
  QPoint wndPos;
  bool mousePressed;
};

#endif
