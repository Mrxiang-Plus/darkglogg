/*
 * Copyright (C) 2009, 2010, 2011, 2013 Nicolas Bonnefon and other contributors
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

#include <QFileDialog>
#include <QtGui>
#include <QLocale>

#include "optionsdialog.h"

#include "configuration.h"
#include "log.h"
#include "persistentinfo.h"
#include "thememanager.h"

static const uint32_t POLL_INTERVAL_MIN = 10;
static const uint32_t POLL_INTERVAL_MAX = 3600000;

// Constructor
OptionsDialog::OptionsDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  setupTabs();
  setupLanguage();
  setupFontList();
  setupRegexp();
  setupThemes();

  // Validators
  QValidator* polling_interval_validator_ =
      new QIntValidator(POLL_INTERVAL_MIN, POLL_INTERVAL_MAX, this);
  pollIntervalLineEdit->setValidator(polling_interval_validator_);

  connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this,
          SLOT(onButtonBoxClicked(QAbstractButton*)));
  connect(fontFamilyBox, SIGNAL(currentIndexChanged(const QString&)), this,
          SLOT(updateFontSize(const QString&)));
  connect(unzipPathButton, SIGNAL(clicked()), this,
          SLOT(onPathButtonClicked()));
  connect(incrementalCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onIncrementalChanged()));
  connect(pollingCheckBox, SIGNAL(toggled(bool)), this,
          SLOT(onPollingChanged()));

  // Theme combo box and radio button mutual exclusion
  connect(themeComboBox, QOverload<int>::of(&QComboBox::activated), [this](int) {
    radioButton_2->setChecked(false);
  });
  connect(radioButton_2, &QRadioButton::toggled, [this](bool checked) {
    if (checked) themeComboBox->setCurrentIndex(-1);
  });

  updateDialogFromConfig();

  setupIncremental();
  setupPolling();
}

//
// Private functions
//

// Setups the tabs depending on the configuration
void OptionsDialog::setupTabs() {
#ifndef GLOGG_SUPPORTS_POLLING
  tabWidget->removeTab(1);
#endif
}

// Populates the 'family' ComboBox
void OptionsDialog::setupFontList() {
  QFontDatabase database;

  // We only show the fixed fonts
  foreach (const QString& str, database.families()) {
    if (database.isFixedPitch(str)) fontFamilyBox->addItem(str);
  }
}

// Populate the regexp ComboBoxes
void OptionsDialog::setupRegexp() {
  QStringList regexpTypes;

  regexpTypes << tr("Extended Regexp") << tr("Fixed Strings");

  mainSearchBox->addItems(regexpTypes);
  quickFindSearchBox->addItems(regexpTypes);
}

// Enable/disable the QuickFind options depending on the state
// of the "incremental" checkbox.
void OptionsDialog::setupIncremental() {
  if (incrementalCheckBox->isChecked()) {
    quickFindSearchBox->setCurrentIndex(getRegexpIndex(FixedString));
    quickFindSearchBox->setEnabled(false);
  } else {
    quickFindSearchBox->setEnabled(true);
  }
}

void OptionsDialog::setupPolling() {
  pollIntervalLineEdit->setEnabled(pollingCheckBox->isChecked());
}

void OptionsDialog::setupLanguage() {
  languageComboBox->addItem("English", "en");
  languageComboBox->addItem(QString::fromUtf8("\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87"), "zh_CN");
}

void OptionsDialog::setupThemes() {
  ThemeManager& tm = ThemeManager::instance();
  QStringList themes = tm.builtinThemes();
  for (const QString& path : themes) {
    QString name = tm.themeName(path);
    themeComboBox->addItem(name, path);
  }
}

// Convert a regexp type to its index in the list
int OptionsDialog::getRegexpIndex(SearchRegexpType syntax) const {
  int index;

  switch (syntax) {
    case FixedString:
      index = 1;
      break;
    default:
      index = 0;
      break;
  }

  return index;
}

// Convert the index of a regexp type to its type
SearchRegexpType OptionsDialog::getRegexpTypeFromIndex(int index) const {
  SearchRegexpType type;

  switch (index) {
    case 1:
      type = FixedString;
      break;
    default:
      type = ExtendedRegexp;
      break;
  }

  return type;
}

// Updates the dialog box using values in global Config()
void OptionsDialog::updateDialogFromConfig() {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");

  // Main font
  QFontInfo fontInfo = QFontInfo(config->mainFont());

  int familyIndex = fontFamilyBox->findText(fontInfo.family());
  if (familyIndex != -1) fontFamilyBox->setCurrentIndex(familyIndex);

  int sizeIndex = fontSizeBox->findText(QString::number(fontInfo.pointSize()));
  if (sizeIndex != -1) fontSizeBox->setCurrentIndex(sizeIndex);
  urlEditLine->setText(config->repoUrl());

  unzipPathEdit->setText(config->unzipPath());
  processFilterEdit->setText(config->processFilter());
  processFilterEdit->hide();
  highlightEdit->setText(config->highlightString());
  // Regexp types
  mainSearchBox->setCurrentIndex(getRegexpIndex(config->mainRegexpType()));
  quickFindSearchBox->setCurrentIndex(
      getRegexpIndex(config->quickfindRegexpType()));

  incrementalCheckBox->setChecked(config->isQuickfindIncremental());

  // Polling
  pollingCheckBox->setChecked(config->pollingEnabled());
  pollIntervalLineEdit->setText(QString::number(config->pollIntervalMs()));
  horizontalSlider->setValue(config->transparent());

  // Last session
  loadLastSessionCheckBox->setChecked(config->loadLastSession());
  updateCheckBox->setChecked(config->loadCheckUpdate());

  // Theme selection
  int activeIdx = config->activeThemeIndex();
  radioButton_2->setChecked(activeIdx == 1);
  if (activeIdx == 0) {
    // Built-in dark theme: select in combo box
    int idx = themeComboBox->findData(config->themePath());
    if (idx >= 0) themeComboBox->setCurrentIndex(idx);
    else themeComboBox->setCurrentIndex(0);
  }

  // Language
  QString lang = config->language();
  if (lang.isEmpty())
    lang = QLocale::system().name();
  int langIndex = languageComboBox->findData(lang);
  if (langIndex == -1)
    langIndex = 0; // Default to English
  languageComboBox->setCurrentIndex(langIndex);
}

//
// Slots
//

void OptionsDialog::updateFontSize(const QString& fontFamily) {
  QFontDatabase database;
  QString oldFontSize = fontSizeBox->currentText();
  QList<int> sizes = database.pointSizes(fontFamily, "");

  fontSizeBox->clear();
  foreach (int size, sizes) { fontSizeBox->addItem(QString::number(size)); }
  // Now restore the size we had before
  int i = fontSizeBox->findText(oldFontSize);
  if (i != -1) fontSizeBox->setCurrentIndex(i);
}

void OptionsDialog::updateConfigFromDialog() {
  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");

  QFont font =
      QFont(fontFamilyBox->currentText(), (fontSizeBox->currentText()).toInt());
  config->setMainFont(font);
  config->setRepoUrl(urlEditLine->text());
  config->setUnzipPath(unzipPathEdit->text());
  config->setProcessFilter(processFilterEdit->text());
  config->setHighlightString(highlightEdit->text());

  config->setMainRegexpType(
      getRegexpTypeFromIndex(mainSearchBox->currentIndex()));
  config->setQuickfindRegexpType(
      getRegexpTypeFromIndex(quickFindSearchBox->currentIndex()));
  config->setQuickfindIncremental(incrementalCheckBox->isChecked());

  config->setPollingEnabled(pollingCheckBox->isChecked());
  uint32_t poll_interval = pollIntervalLineEdit->text().toUInt();
  uint32_t transparent = horizontalSlider->value();

  if (poll_interval < POLL_INTERVAL_MIN)
    poll_interval = POLL_INTERVAL_MIN;
  else if (poll_interval > POLL_INTERVAL_MAX)
    poll_interval = POLL_INTERVAL_MAX;

  config->setPollIntervalMs(poll_interval);
  config->setTransparent(transparent);

  config->setLoadLastSession(loadLastSessionCheckBox->isChecked());
  config->setCheckUpdate(updateCheckBox->isChecked());

  // Theme selection
  if (radioButton_2->isChecked()) {
    // White theme
    config->setActiveThemeIndex(1);
    config->setWasdStyle(false);
    config->setCustomChecked(false);
  } else {
    // Built-in dark theme from combo box
    QString themePath = themeComboBox->currentData().toString();
    config->setThemePath(themePath);
    config->setActiveThemeIndex(0);
    config->setWasdStyle(true);
    config->setCustomChecked(false);
    // Load the theme immediately
    ThemeManager::instance().loadTheme(themePath);
  }

  // Language
  QString newLang = languageComboBox->currentData().toString();
  if (newLang != config->language()) {
    config->setLanguage(newLang);
    emit languageChanged();
  }

  emit optionsChanged();
}

void OptionsDialog::onButtonBoxClicked(QAbstractButton* button) {
  QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
  if ((role == QDialogButtonBox::AcceptRole) ||
      (role == QDialogButtonBox::ApplyRole)) {
    updateConfigFromDialog();
  }

  if (role == QDialogButtonBox::AcceptRole)
    accept();
  else if (role == QDialogButtonBox::RejectRole)
    reject();
}

void OptionsDialog::onPathButtonClicked() {
  QString filePath =
      QFileDialog::getExistingDirectory(this, tr("Select Unzip path"));

  unzipPathEdit->setText(filePath);
}

void OptionsDialog::onIncrementalChanged() { setupIncremental(); }

void OptionsDialog::onPollingChanged() { setupPolling(); }
