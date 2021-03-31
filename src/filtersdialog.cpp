/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
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

#include "log.h"

#include "configuration.h"
#include "filterset.h"
#include "frqfilterset.h"
#include "persistentinfo.h"

#include "filtersdialog.h"

static const QString DEFAULT_DESCRIPTION = "Pattern";
static const QString DEFAULT_PATTERN = "New Filter";
static const bool DEFAULT_IGNORE_CASE = false;
static const QString DEFAULT_FORE_COLOUR = "silver";
static const QString DEFAULT_BACK_COLOUR = "window";

// Construct the box, including a copy of the global FilterSet
// to handle ok/cancel/apply
FiltersDialog::FiltersDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);

  // Reload the filter list from disk (in case it has been changed
  // by another glogg instance) and copy it to here.
  GetPersistentInfo().retrieve("filterSet");
  filterSet = PersistentCopy<FilterSet>("filterSet");
  GetPersistentInfo().retrieve("frqFilterSet");
  frqFilterSet = PersistentCopy<FrqFilterSet>("frqFilterSet");

  populateColors();
  populateFilterList();

  // Start with all buttons disabled except 'add'
  removeFilterButton->setEnabled(false);
  upFilterButton->setEnabled(false);
  downFilterButton->setEnabled(false);

  // Default to black on white
  int index = foreColorBox->findText(DEFAULT_FORE_COLOUR);
  foreColorBox->setCurrentIndex(index);
  index = backColorBox->findText(DEFAULT_BACK_COLOUR);
  backColorBox->setCurrentIndex(index);

  // No filter selected by default
  selectedRow_ = -1;

  connect(filterListWidget, SIGNAL(itemSelectionChanged()), this,
          SLOT(updatePropertyFields()));
  connect(pinnedListWidget, SIGNAL(itemSelectionChanged()), this,
          SLOT(updatePropertyFields()));
  connect(descriptionEdit, SIGNAL(textEdited(const QString&)), this,
          SLOT(updateFilterProperties()));
  connect(patternEdit, SIGNAL(textEdited(const QString&)), this,
          SLOT(updateFilterProperties()));
  connect(ignoreCaseCheckBox, SIGNAL(clicked(bool)), this,
          SLOT(updateFilterProperties()));
  connect(foreColorBox, SIGNAL(activated(int)), this,
          SLOT(updateFilterProperties()));
  connect(backColorBox, SIGNAL(activated(int)), this,
          SLOT(updateFilterProperties()));

  if (!filterSet->filterList.empty()) {
    filterListWidget->setCurrentItem(filterListWidget->item(0));
  }
  if (!frqFilterSet->frqFilterList.empty()) {
    pinnedListWidget->setCurrentItem(pinnedListWidget->item(0));
  }
}

//
// Slots
//

void FiltersDialog::on_addFilterButton_clicked() {
  LOG(logDEBUG) << "on_addFilterButton_clicked()";

  Filter newFilter =
      Filter(DEFAULT_DESCRIPTION, DEFAULT_PATTERN, DEFAULT_IGNORE_CASE,
             DEFAULT_FORE_COLOUR, DEFAULT_BACK_COLOUR);
  filterSet->filterList << newFilter;

  // Add and select the newly created filter
  filterListWidget->addItem(DEFAULT_PATTERN);
  filterListWidget->setCurrentRow(filterListWidget->count() - 1);
}

void FiltersDialog::on_removeFilterButton_clicked() {
  int index = filterListWidget->currentRow();
  LOG(logDEBUG) << "on_removeFilterButton_clicked() index " << index;

  if (index >= 0) {
    filterSet->filterList.removeAt(index);
    filterListWidget->setCurrentRow(-1);
    delete filterListWidget->takeItem(index);

    int count = filterListWidget->count();
    if (index < count) {
      // Select the new item at the same index
      filterListWidget->setCurrentRow(index);
    } else {
      // or the previous index if it is at the end
      filterListWidget->setCurrentRow(count - 1);
    }
  }
}

void FiltersDialog::on_upFilterButton_clicked() {
  int index = filterListWidget->currentRow();
  LOG(logDEBUG) << "on_upFilterButton_clicked() index " << index;

  if (index > 0) {
    filterSet->filterList.move(index, index - 1);

    QListWidgetItem* item = filterListWidget->takeItem(index);
    filterListWidget->insertItem(index - 1, item);
    filterListWidget->setCurrentRow(index - 1);
  }
}

void FiltersDialog::on_downFilterButton_clicked() {
  int index = filterListWidget->currentRow();
  LOG(logDEBUG) << "on_downFilterButton_clicked() index " << index;

  if ((index >= 0) && (index < (filterListWidget->count() - 1))) {
    filterSet->filterList.move(index, index + 1);

    QListWidgetItem* item = filterListWidget->takeItem(index);
    filterListWidget->insertItem(index + 1, item);
    filterListWidget->setCurrentRow(index + 1);
  }
}

void FiltersDialog::on_buttonBox_clicked(QAbstractButton* button) {
  LOG(logDEBUG) << "on_buttonBox_clicked()";

  QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
  if ((role == QDialogButtonBox::AcceptRole) ||
      (role == QDialogButtonBox::ApplyRole)) {
    // Copy the filter set and persist it to disk
    *(Persistent<FilterSet>("filterSet")) = *filterSet;
    GetPersistentInfo().save("filterSet");
    *(Persistent<FrqFilterSet>("frqFilterSet")) = *frqFilterSet;
    GetPersistentInfo().save("frqFilterSet");
    emit optionsChanged();
  }

  if (role == QDialogButtonBox::AcceptRole)
    accept();
  else if (role == QDialogButtonBox::RejectRole)
    reject();
}

void FiltersDialog::updatePropertyFields() {
  if (pinnedListWidget->hasFocus()) {
    focusedListWidget_ = pinnedListWidget;
  } else {
    focusedListWidget_ = filterListWidget;
  }

  if (focusedListWidget_->selectedItems().count() >= 1)
    selectedRow_ =
        focusedListWidget_->row(focusedListWidget_->selectedItems().at(0));
  else
    selectedRow_ = -1;

  LOG(logDEBUG) << "updatePropertyFields(), row = " << selectedRow_;

  if (selectedRow_ >= 0) {
    int index = 0;
    if (pinnedListWidget->hasFocus()) {
      const FrqFilter& currentFilter =
          frqFilterSet->frqFilterList.at(selectedRow_);

      patternEdit->setText(currentFilter.pattern());
      patternEdit->setEnabled(true);
      descriptionEdit->setText(currentFilter.description());
      descriptionEdit->setEnabled(true);

      ignoreCaseCheckBox->setChecked(currentFilter.ignoreCase());
      ignoreCaseCheckBox->setEnabled(true);

      index = foreColorBox->findText(currentFilter.foreColorName());
      if (index != -1) {
        LOG(logDEBUG) << "fore index = " << index;
        foreColorBox->setCurrentIndex(index);
        foreColorBox->setEnabled(true);
      }
      index = backColorBox->findText(currentFilter.backColorName());
    } else {
      const Filter& currentFilter = filterSet->filterList.at(selectedRow_);

      patternEdit->setText(currentFilter.pattern());
      patternEdit->setEnabled(true);
      descriptionEdit->setText(currentFilter.description());
      descriptionEdit->setEnabled(true);

      ignoreCaseCheckBox->setChecked(currentFilter.ignoreCase());
      ignoreCaseCheckBox->setEnabled(true);

      index = foreColorBox->findText(currentFilter.foreColorName());
      if (index != -1) {
        LOG(logDEBUG) << "fore index = " << index;
        foreColorBox->setCurrentIndex(index);
        foreColorBox->setEnabled(true);
      }
      index = backColorBox->findText(currentFilter.backColorName());
    }

    if (index != -1) {
      LOG(logDEBUG) << "back index = " << index;
      backColorBox->setCurrentIndex(index);
      backColorBox->setEnabled(true);
    }

    // Enable the buttons if needed
    removeFilterButton->setEnabled(true);
    upFilterButton->setEnabled(selectedRow_ > 0);
    downFilterButton->setEnabled(selectedRow_ <
                                 (focusedListWidget_->count() - 1));
  } else {
    // Nothing is selected, reset and disable the controls
    patternEdit->clear();
    patternEdit->setEnabled(false);
    descriptionEdit->clear();
    descriptionEdit->setEnabled(false);

    int index = foreColorBox->findText(DEFAULT_FORE_COLOUR);
    foreColorBox->setCurrentIndex(index);
    foreColorBox->setEnabled(false);
    index = backColorBox->findText(DEFAULT_BACK_COLOUR);

    backColorBox->setCurrentIndex(index);
    backColorBox->setEnabled(false);

    ignoreCaseCheckBox->setChecked(DEFAULT_IGNORE_CASE);
    ignoreCaseCheckBox->setEnabled(false);
    removeFilterButton->setEnabled(false);
    upFilterButton->setEnabled(false);
    downFilterButton->setEnabled(false);
  }
}

void FiltersDialog::updateFilterProperties() {
  LOG(logDEBUG) << "updateFilterProperties()";

  // If a row is selected
  if (selectedRow_ >= 0) {
    if (focusedListWidget_ == filterListWidget) {
      Filter& currentFilter = filterSet->filterList[selectedRow_];

      // Update the internal data
      currentFilter.setPattern(patternEdit->text());
      currentFilter.setIgnoreCase(ignoreCaseCheckBox->isChecked());
      currentFilter.setForeColor(foreColorBox->currentText());
      currentFilter.setBackColor(backColorBox->currentText());
      currentFilter.setDescription(descriptionEdit->text());

      // Update the entry in the filterList widget
      filterListWidget->currentItem()->setText(patternEdit->text());
      filterListWidget->currentItem()->setForeground(
          QBrush(currentFilter.foreColor()));
      filterListWidget->currentItem()->setBackground(
          QBrush(currentFilter.backColor()));
    } else {
      FrqFilter& currentFilter = frqFilterSet->frqFilterList[selectedRow_];

      // Update the internal data
      currentFilter.setPattern(patternEdit->text());
      currentFilter.setIgnoreCase(ignoreCaseCheckBox->isChecked());
      currentFilter.setForeColor(foreColorBox->currentText());
      currentFilter.setBackColor(backColorBox->currentText());
      currentFilter.setDescription(descriptionEdit->text());
      // Update the entry in the filterList widget
      pinnedListWidget->currentItem()->setText(patternEdit->text());
      pinnedListWidget->currentItem()->setForeground(
          QBrush(currentFilter.foreColor()));
      pinnedListWidget->currentItem()->setBackground(
          QBrush(currentFilter.backColor()));
    }
  }
}

//
// Private functions
//

// Fills the color selection combo boxes
void FiltersDialog::populateColors() {
  const QStringList colorNames = QStringList()
                                 // Basic 16 HTML colors (minus greys):
                                 << "black"
                                 << "white"
                                 << "maroon"
                                 << "red"
                                 << "purple"
                                 << "fuchsia"
                                 << "green"
                                 << "lime"
                                 << "olive"
                                 << "yellow"
                                 << "navy"
                                 << "blue"
                                 << "teal"
                                 << "aqua"
                                 // Greys
                                 << "gainsboro"
                                 << "lightgrey"
                                 << "silver"
                                 << "darkgrey"
                                 << "grey"
                                 << "dimgrey"
                                 // Reds
                                 << "tomato"
                                 << "orangered"
                                 << "orange"
                                 << "crimson"
                                 << "darkred"
                                 // Greens
                                 << "greenyellow"
                                 << "lightgreen"
                                 << "darkgreen"
                                 << "lightseagreen"
                                 // Blues
                                 << "lightcyan"
                                 << "darkturquoise"
                                 << "steelblue"
                                 << "lightblue"
                                 << "royalblue"
                                 << "darkblue"
                                 << "midnightblue"
                                 // Browns
                                 << "bisque"
                                 << "tan"
                                 << "sandybrown"
                                 << "chocolate";

  QPixmap solidPixmap(20, 10);

  std::shared_ptr<Configuration> config = Persistent<Configuration>("settings");
  if (config->wasdStyle()) {
    solidPixmap.fill(QColor(33, 33, 33));
  } else {
    solidPixmap.fill(QColor(239, 235, 231));
  }

  QIcon solidIcon{solidPixmap};

  foreColorBox->addItem(solidIcon, "window");
  backColorBox->addItem(solidIcon, "window");

  QPixmap solidPixmap2(20, 10);
  solidPixmap2.fill(QColor(255, 203, 107));
  QIcon solidIcon2{solidPixmap2};

  foreColorBox->addItem(solidIcon2, "text");
  backColorBox->addItem(solidIcon2, "text");
  for (QStringList::const_iterator i = colorNames.constBegin();
       i != colorNames.constEnd(); ++i) {
    QPixmap solidPixmap(20, 10);
    solidPixmap.fill(QColor(*i));
    QIcon solidIcon{solidPixmap};

    foreColorBox->addItem(solidIcon, *i);
    backColorBox->addItem(solidIcon, *i);
  }
}

void FiltersDialog::populateFilterList() {
  filterListWidget->clear();
  foreach (Filter filter, filterSet->filterList) {
    QListWidgetItem* new_item = new QListWidgetItem(filter.pattern());
    // new_item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable |
    // Qt::ItemIsEnabled );
    new_item->setForeground(QBrush(filter.foreColor()));
    new_item->setBackground(QBrush(filter.backColor()));
    filterListWidget->addItem(new_item);
  }
  pinnedListWidget->clear();
  foreach (FrqFilter filter, frqFilterSet->frqFilterList) {
    QListWidgetItem* new_item = new QListWidgetItem(filter.pattern());
    // new_item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable |
    // Qt::ItemIsEnabled );
    new_item->setForeground(QBrush(filter.foreColor()));
    new_item->setBackground(QBrush(filter.backColor()));
    pinnedListWidget->addItem(new_item);
  }
}

void FiltersDialog::on_pinnedButton_clicked() {
  LOG(logDEBUG) << "on_addFilterButton_clicked()";

  if (selectedRow_ >= 0) {
    const Filter& currentFilter = filterSet->filterList.at(selectedRow_);
    FrqFilter newFilter =
        FrqFilter(currentFilter.description(), currentFilter.pattern(),
                  currentFilter.ignoreCase(), currentFilter.foreColorName(),
                  currentFilter.backColorName());
    bool isContained = false;
    int i = 0;
    foreach (FrqFilter filter, frqFilterSet->frqFilterList) {
      if (filter.pattern() == newFilter.pattern()) {
        isContained = true;
        break;
      }
      i++;
    }

    if (!isContained) {
      frqFilterSet->frqFilterList << newFilter;
      pinnedListWidget->addItem(currentFilter.pattern());
      pinnedListWidget->setCurrentRow(pinnedListWidget->count() - 1);

      pinnedListWidget->currentItem()->setForeground(
          QBrush(currentFilter.foreColor()));
      pinnedListWidget->currentItem()->setBackground(
          QBrush(currentFilter.backColor()));
    } else {
      pinnedListWidget->setCurrentRow(i);
    }
  }

  //  patternEdit->setText(currentFilter.pattern());
  //  ignoreCaseCheckBox->setChecked(currentFilter.ignoreCase());
  //  int index = foreColorBox->findText(currentFilter.foreColorName());
  //  if (index != -1) {
  //    LOG(logDEBUG) << "fore index = " << index;
  //    foreColorBox->setCurrentIndex(index);
  //    foreColorBox->setEnabled(true);
  //  }
  //  index = backColorBox->findText(currentFilter.backColorName());

  // Add and select the newly created filter
}

void FiltersDialog::on_removeFilterButton_4_clicked() {
  int index = pinnedListWidget->currentRow();
  LOG(logDEBUG) << "on_removeFilterButton_clicked() index " << index;

  if (index >= 0) {
    frqFilterSet->frqFilterList.removeAt(index);
    pinnedListWidget->setCurrentRow(-1);
    delete pinnedListWidget->takeItem(index);

    int count = pinnedListWidget->count();
    if (index < count) {
      // Select the new item at the same index
      pinnedListWidget->setCurrentRow(index);
    } else {
      // or the previous index if it is at the end
      pinnedListWidget->setCurrentRow(count - 1);
    }
  }
}

void FiltersDialog::on_addFilterButton_4_clicked() {
  LOG(logDEBUG) << "on_addFilterButton_clicked()";

  FrqFilter newFilter =
      FrqFilter(DEFAULT_DESCRIPTION, DEFAULT_PATTERN, DEFAULT_IGNORE_CASE,
                DEFAULT_FORE_COLOUR, DEFAULT_BACK_COLOUR);
  frqFilterSet->frqFilterList << newFilter;

  // Add and select the newly created filter
  pinnedListWidget->addItem(DEFAULT_PATTERN);
  pinnedListWidget->setCurrentRow(filterListWidget->count() - 1);
}

void FiltersDialog::on_upFilterButton_4_clicked() {
  int index = pinnedListWidget->currentRow();
  LOG(logDEBUG) << "on_upFilterButton_clicked() index " << index;

  if (index > 0) {
    frqFilterSet->frqFilterList.move(index, index - 1);

    QListWidgetItem* item = pinnedListWidget->takeItem(index);
    pinnedListWidget->insertItem(index - 1, item);
    pinnedListWidget->setCurrentRow(index - 1);
  }
}

void FiltersDialog::on_downFilterButton_4_clicked() {
  int index = pinnedListWidget->currentRow();
  LOG(logDEBUG) << "on_downFilterButton_clicked() index " << index;

  if ((index >= 0) && (index < (pinnedListWidget->count() - 1))) {
    frqFilterSet->frqFilterList.move(index, index + 1);

    QListWidgetItem* item = pinnedListWidget->takeItem(index);
    pinnedListWidget->insertItem(index + 1, item);
    pinnedListWidget->setCurrentRow(index + 1);
  }
}

void FiltersDialog::on_filterListWidget_clicked(const QModelIndex& index) {
  focusedListWidget_ = filterListWidget;
  QString st;
  st.append("QListWidget:item { selection-background-color: gray; }");
  filterListWidget->setStyleSheet("");
  pinnedListWidget->setStyleSheet(st);
  selectedRow_ =
      focusedListWidget_->row(focusedListWidget_->selectedItems().at(0));
}

void FiltersDialog::on_pinnedListWidget_clicked(const QModelIndex& index) {
  focusedListWidget_ = pinnedListWidget;
  QString st;
  st.append("QListWidget:item { selection-background-color: gray; }");
  pinnedListWidget->setStyleSheet("");
  filterListWidget->setStyleSheet(st);
  selectedRow_ =
      focusedListWidget_->row(focusedListWidget_->selectedItems().at(0));
}
