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
    palette.setColor(QPalette::Window, QColor(33, 33, 33, alpha_));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                     QColor(127, 127, 127));
    palette.setColor(QPalette::Base, QColor(33, 33, 33, alpha_));
    palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    palette.setColor(QPalette::ToolTipBase, Qt::gray);
    palette.setColor(QPalette::ToolTipText, QColor(33, 33, 33, alpha_));
    palette.setColor(QPalette::Text, Qt::gray);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Dark, QColor(35, 35, 35));
    palette.setColor(QPalette::Shadow, QColor(33, 33, 33, alpha_));
    palette.setColor(QPalette::Button, QColor(33, 33, 33, alpha_));
    palette.setColor(QPalette::ButtonText, Qt::gray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     QColor(127, 127, 127));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::Disabled, QPalette::Highlight,
                     QColor(80, 80, 80));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                     QColor(127, 127, 127));
    palette.setColor(QPalette::NoRole, QColor(70, 70, 70));
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
