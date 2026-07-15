/*
###############################################################################
#                                                                             #
# The MIT License                                                             #
#                                                                             #
# Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)          #
#               >> https://github.com/Jorgen-VikingGod                        #
#                                                                             #
# Sources: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle  #
#                                                                             #
###############################################################################
*/

#include "DarkStyle.h"
#include "configuration.h"
#include "log.h"
#include "persistentinfo.h"

DarkStyle::DarkStyle(uint32_t alpha) : DarkStyle(styleBase()) {
  alpha_ = alpha;
}

DarkStyle::DarkStyle(QStyle *style) : QProxyStyle(style) {}

QStyle *DarkStyle::styleBase(QStyle *style) const {
  static QStyle *base =
      !style ? QStyleFactory::create(QStringLiteral("Fusion")) : style;
  return base;
}

QStyle *DarkStyle::baseStyle() const { return styleBase(); }

void DarkStyle::polish(QPalette &palette) {
  // modify palette to dark

  std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");
  if (config->wasdStyle()) {
    // VSCode Dark+ inspired palette
    palette.setColor(QPalette::Window, QColor(30, 30, 30, alpha_));           // #1e1e1e
    palette.setColor(QPalette::WindowText, QColor(212, 212, 212));            // #d4d4d4
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                     QColor(100, 100, 100));
    palette.setColor(QPalette::Base, QColor(30, 30, 30, alpha_));             // #1e1e1e editor bg
    palette.setColor(QPalette::AlternateBase, QColor(38, 38, 38));            // #262626
    palette.setColor(QPalette::ToolTipBase, QColor(37, 37, 38));              // #252526
    palette.setColor(QPalette::ToolTipText, QColor(204, 204, 204));           // #cccccc
    palette.setColor(QPalette::Text, QColor(204, 204, 204));                  // #cccccc
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(100, 100, 100));
    palette.setColor(QPalette::Dark, QColor(45, 45, 45));                     // #2d2d2d
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));                   // #141414
    palette.setColor(QPalette::Button, QColor(45, 45, 45, alpha_));           // #2d2d2d
    palette.setColor(QPalette::ButtonText, QColor(212, 212, 212));            // #d4d4d4
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     QColor(100, 100, 100));
    palette.setColor(QPalette::BrightText, QColor(255, 85, 85));              // #ff5555
    palette.setColor(QPalette::Link, QColor(79, 193, 255));                   // #4fc1ff
    palette.setColor(QPalette::Highlight, QColor(0, 122, 204));               // #007acc VSCode blue
    palette.setColor(QPalette::Disabled, QPalette::Highlight,
                     QColor(60, 60, 60));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                     QColor(100, 100, 100));
    palette.setColor(QPalette::NoRole, QColor(60, 60, 60));                   // #3c3c3c
  } else if (config->wasCustomStyle() && config->getCustomStyle() != NULL){
      palette.setColor(QPalette::Window, QColor(config->getCustomStyle().right(6).toUInt(NULL, 16)));
      palette.setColor(QPalette::Window, QColor(config->getCustomStyle().right(6).toUInt(NULL, 16)));
  }
  else {
      palette.setColor(QPalette::NoRole, QColor(127, 127, 127));
  }
}

void DarkStyle::polish(QApplication *app) {
  if (!app) return;

  // increase font size for better reading,
  // setPointSize was reduced from +2 because when applied this way in Qt5, the
  // font is larger than intended for some reason
  QFont defaultFont = QApplication::font();
  defaultFont.setFamily("Consolas");
  defaultFont.setPointSize(defaultFont.pointSize());
  app->setFont(defaultFont);

  std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");
  if (config->wasdStyle()) {
    // loadstylesheet
    QFile qfDarkstyle(QStringLiteral(":/darkstyle/darkstyle.qss"));
    if (qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
      // set stylesheet
      QString qsStylesheet = QString::fromLatin1(qfDarkstyle.readAll());
      app->setStyleSheet(qsStylesheet);
      qfDarkstyle.close();
    }
  }
}
