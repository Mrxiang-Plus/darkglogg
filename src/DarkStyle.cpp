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
#include "thememanager.h"

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
  ThemeManager& tm = ThemeManager::instance();

  if (tm.currentThemePath().isEmpty()) {
    palette.setColor(QPalette::Window, QColor(30, 30, 30, alpha_));
    palette.setColor(QPalette::WindowText, QColor(212, 212, 212));
    palette.setColor(QPalette::Base, QColor(30, 30, 30, alpha_));
    palette.setColor(QPalette::Highlight, QColor(0, 122, 204));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Text, QColor(204, 204, 204));
    palette.setColor(QPalette::Button, QColor(45, 45, 45, alpha_));
    palette.setColor(QPalette::ButtonText, QColor(212, 212, 212));
    return;
  }

  QColor disabled = QColor(100, 100, 100);

  // Editor colors
  QColor editorBg = tm.color("editor.background", QColor("#1e1e1e"));
  QColor editorFg = tm.color("editor.foreground", QColor("#d4d4d4"));
  QColor selection = tm.color("editor.selectionBackground", QColor("#264f78"));

  // Sidebar
  QColor sidebarBg = tm.color("sidebar.background", QColor("#252526"));
  QColor sidebarFg = tm.color("sidebar.foreground", QColor("#cccccc"));

  // Title bar
  QColor titleBg = tm.color("titleBar.background", QColor("#323233"));
  QColor titleFg = tm.color("titleBar.foreground", QColor("#cccccc"));

  // Status bar
  QColor statusBg = tm.color("statusBar.background", QColor("#007acc"));
  QColor statusFg = tm.color("statusBar.foreground", QColor("#ffffff"));

  // Tabs
  QColor tabActiveBg = tm.color("tab.activeBackground", editorBg);
  QColor tabInactiveBg = tm.color("tab.inactiveBackground", QColor("#2d2d2d"));

  // Menu
  QColor menuBg = tm.color("menu.background", sidebarBg);
  QColor menuFg = tm.color("menu.foreground", sidebarFg);
  QColor menuSel = tm.color("menu.selectionBackground", QColor("#094771"));

  // Buttons
  QColor btnBg = tm.color("button.background", QColor("#0e639c"));
  QColor btnFg = tm.color("button.foreground", QColor("#ffffff"));

  // Inputs
  QColor inputBg = tm.color("input.background", QColor("#3c3c3c"));
  QColor inputFg = tm.color("input.foreground", QColor("#cccccc"));

  // Scrollbar
  QColor scrollBg = tm.color("scrollbarSlider.background", QColor("#5a5a5a"));

  // Tooltip
  QColor tooltipBg = tm.color("tooltip.background", sidebarBg);
  QColor tooltipFg = tm.color("tooltip.foreground", sidebarFg);

  // General
  QColor border = tm.color("border", QColor("#3c3c3c"));
  QColor accent = tm.color("accent", QColor("#007acc"));
  QColor link = tm.color("link", QColor("#4fc1ff"));
  QColor error = tm.color("error", QColor("#ff5555"));

  // Apply palette with alpha for backgrounds
  palette.setColor(QPalette::Window, QColor(editorBg.red(), editorBg.green(), editorBg.blue(), alpha_));
  palette.setColor(QPalette::WindowText, editorFg);
  palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
  palette.setColor(QPalette::Base, QColor(editorBg.red(), editorBg.green(), editorBg.blue(), alpha_));
  palette.setColor(QPalette::AlternateBase, sidebarBg);
  palette.setColor(QPalette::ToolTipBase, tooltipBg);
  palette.setColor(QPalette::ToolTipText, tooltipFg);
  palette.setColor(QPalette::Text, inputFg);
  palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
  palette.setColor(QPalette::Dark, tabInactiveBg);
  palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  palette.setColor(QPalette::Button, QColor(btnBg.red(), btnBg.green(), btnBg.blue(), alpha_));
  palette.setColor(QPalette::ButtonText, btnFg);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
  palette.setColor(QPalette::BrightText, error);
  palette.setColor(QPalette::Link, link);
  palette.setColor(QPalette::Highlight, accent);
  palette.setColor(QPalette::Disabled, QPalette::Highlight, border);
  palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
  palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
  palette.setColor(QPalette::NoRole, border);
}

void DarkStyle::polish(QApplication *app) {
  if (!app) return;

  QFont defaultFont = QApplication::font();
  defaultFont.setFamily("Consolas");
  defaultFont.setPointSize(defaultFont.pointSize());
  app->setFont(defaultFont);

  // Load dark stylesheet for dark themes
  std::shared_ptr<Configuration> config =
      Persistent<Configuration>("settings");
  int activeIdx = config->activeThemeIndex();
  if (activeIdx == 0 || activeIdx >= 2) {
    QFile qfDarkstyle(QStringLiteral(":/darkstyle/darkstyle.qss"));
    if (qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString qsStylesheet = QString::fromLatin1(qfDarkstyle.readAll());

      // Inject theme colors into stylesheet via dynamic properties
      ThemeManager& tm = ThemeManager::instance();
      QColor accent = tm.color("accent", QColor("#007acc"));
      QColor inputBg = tm.color("input.background", QColor("#3c3c3c"));
      QColor inputBorder = tm.color("input.border", QColor("#3c3c3c"));
      QColor focusBorder = tm.color("input.focusBorder", accent);
      QColor menuSel = tm.color("menu.selectionBackground", QColor("#094771"));
      QColor scrollHover = tm.color("scrollbarSlider.hoverBackground", QColor("#7a7a7a"));
      QColor tabBorder = tm.color("tab.activeBorder", accent);
      QColor listSel = tm.color("list.selectionBackground", menuSel);
      QColor listHover = tm.color("list.hoverBackground", QColor("#2a2d2e"));
      QColor btnBg = tm.color("button.background", QColor("#0e639c"));
      QColor btnHover = tm.color("button.hoverBackground", QColor("#1177bb"));

      // Append dynamic color overrides
      qsStylesheet += QString("\n"
        "QLineEdit:focus { border: 1px solid %1; }\n"
        "QComboBox:focus { border: 1px solid %1; }\n"
        "QTabBar::tab:selected { border-bottom: 2px solid %2; }\n"
        "QSplitter::handle:hover { background-color: %1; }\n"
        "QScrollBar::handle:vertical:hover { background-color: %3; }\n"
        "QScrollBar::handle:horizontal:hover { background-color: %3; }\n"
        "QMenu::item:selected { background: %4; border-color: %4; }\n"
        "QTreeView::item:selected, QTableView::item:selected { background: %5; }\n"
        "QTreeView::item:hover, QTableView::item:hover { background: %6; }\n"
        "QPushButton { background-color: %7; border: 1px solid %7; }\n"
        "QPushButton:hover { background-color: %8; border: 1px solid %8; }\n"
        "QSlider::handle:horizontal { border: 1px solid %1; }\n"
        "QSlider::sub-page:horizontal { background: %1; }\n"
        "QStatusBar { background-color: %9; color: %10; }\n"
      ).arg(focusBorder.name())
       .arg(tabBorder.name())
       .arg(scrollHover.name())
       .arg(menuSel.name())
       .arg(listSel.name())
       .arg(listHover.name())
       .arg(btnBg.name())
       .arg(btnHover.name())
       .arg(tm.color("statusBar.background", QColor("#007acc")).name())
       .arg(tm.color("statusBar.foreground", QColor("#ffffff")).name());

      app->setStyleSheet(qsStylesheet);
      qfDarkstyle.close();
    }
  }
}
