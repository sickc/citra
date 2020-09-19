// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include "citra_qt/configuration/config.h"
#include "citra_qt/configuration/configure_hotkeys.h"
#include "citra_qt/hotkeys.h"
#include "citra_qt/util/sequence_dialog/sequence_dialog.h"
#include "core/settings.h"
#include "ui_configure_hotkeys.h"

ConfigureHotkeys::ConfigureHotkeys(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureHotkeys>()) {
    ui->setupUi(this);
    setFocusPolicy(Qt::ClickFocus);

    model = new QStandardItemModel(this);
    model->setColumnCount(3);
    model->setHorizontalHeaderLabels({tr("Action"), tr("Hotkey"), tr("Context")});

    connect(ui->hotkey_list, &QTreeView::doubleClicked, this, &ConfigureHotkeys::Configure);
    connect(ui->hotkey_list, &QTreeView::customContextMenuRequested, this,
            &ConfigureHotkeys::PopupContextMenu);
    ui->hotkey_list->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->hotkey_list->setModel(model);

    // TODO(Kloen): Make context configurable as well (hiding the column for now)
    ui->hotkey_list->hideColumn(2);

    ui->hotkey_list->setColumnWidth(0, 200);
    ui->hotkey_list->resizeColumnToContents(1);

    connect(ui->button_restore_defaults, &QPushButton::clicked, this,
            &ConfigureHotkeys::RestoreDefaults);
    connect(ui->button_clear_all, &QPushButton::clicked, this, &ConfigureHotkeys::ClearAll);
}

ConfigureHotkeys::~ConfigureHotkeys() = default;

void ConfigureHotkeys::EmitHotkeysChanged() {
    emit HotkeysChanged(GetUsedKeyList());
}

QList<QKeySequence> ConfigureHotkeys::GetUsedKeyList() const {
    QList<QKeySequence> list;
    for (int r = 0; r < model->rowCount(); r++) {
        QStandardItem* parent = model->item(r, 0);
        for (int r2 = 0; r2 < parent->rowCount(); r2++) {
            QStandardItem* keyseq = parent->child(r2, 1);
            list << QKeySequence::fromString(keyseq->text(), QKeySequence::NativeText);
        }
    }
    return list;
}

void ConfigureHotkeys::Populate(const HotkeyRegistry& registry) {
    for (const auto& group : registry.hotkey_groups) {
        QStandardItem* parent_item = new QStandardItem(group.first);
        parent_item->setEditable(false);
        for (const auto& hotkey : group.second) {
            QStandardItem* action = new QStandardItem(hotkey.first);
            QStandardItem* keyseq =
                new QStandardItem(hotkey.second.keyseq.toString(QKeySequence::NativeText));
            action->setEditable(false);
            keyseq->setEditable(false);
            parent_item->appendRow({action, keyseq});
        }
        model->appendRow(parent_item);
    }

    ui->hotkey_list->expandAll();
}

void ConfigureHotkeys::OnInputKeysChanged(QList<QKeySequence> new_key_list) {
    input_keys_list = new_key_list;
}

void ConfigureHotkeys::Configure(QModelIndex index) {
    if (!index.parent().isValid()) {
        return;
    }

    index = index.sibling(index.row(), 1);
    const auto previous_key = model->data(index);

    SequenceDialog hotkey_dialog{this};

    const int return_code = hotkey_dialog.exec();
    const auto key_sequence = hotkey_dialog.GetSequence();
    if (return_code == QDialog::Rejected || key_sequence.isEmpty()) {
        return;
    }

    if (IsUsedKey(key_sequence) && key_sequence != QKeySequence(previous_key.toString())) {
        QMessageBox::warning(this, tr("Conflicting Key Sequence"),
                             tr("The entered key sequence is already assigned to another hotkey."));
    } else {
        model->setData(index, key_sequence.toString(QKeySequence::NativeText));
        EmitHotkeysChanged();
    }
}

bool ConfigureHotkeys::IsUsedKey(QKeySequence key_sequence) const {
    return input_keys_list.contains(key_sequence) || GetUsedKeyList().contains(key_sequence);
}

void ConfigureHotkeys::ApplyConfiguration(HotkeyRegistry& registry) {
    for (int key_id = 0; key_id < model->rowCount(); key_id++) {
        QStandardItem* parent = model->item(key_id, 0);
        for (int key_column_id = 0; key_column_id < parent->rowCount(); key_column_id++) {
            QStandardItem* action = parent->child(key_column_id, 0);
            QStandardItem* keyseq = parent->child(key_column_id, 1);
            for (auto& [group, sub_actions] : registry.hotkey_groups) {
                if (group != parent->text())
                    continue;
                for (auto& [action_name, hotkey] : sub_actions) {
                    if (action_name != action->text())
                        continue;
                    hotkey.keyseq = QKeySequence(keyseq->text());
                }
            }
        }
    }

    registry.SaveHotkeys();
}

void ConfigureHotkeys::RestoreDefaults() {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* parent = model->item(r, 0);

        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            model->item(r, 0)->child(r2, 1)->setText(Config::default_hotkeys[r2].shortcut.first);
        }
    }
}

void ConfigureHotkeys::ClearAll() {
    for (int r = 0; r < model->rowCount(); ++r) {
        const QStandardItem* parent = model->item(r, 0);

        for (int r2 = 0; r2 < parent->rowCount(); ++r2) {
            model->item(r, 0)->child(r2, 1)->setText(tr(""));
        }
    }
}

void ConfigureHotkeys::PopupContextMenu(const QPoint& menu_location) {
    QModelIndex index = ui->hotkey_list->indexAt(menu_location);
    if (!index.parent().isValid()) {
        return;
    }

    const auto selected = index.sibling(index.row(), 1);
    QMenu context_menu;

    QAction* restore_default = context_menu.addAction(tr("Restore Default"));
    QAction* clear = context_menu.addAction(tr("Clear"));

    connect(restore_default, &QAction::triggered, [this, selected] {
        const QKeySequence& default_key_sequence = QKeySequence::fromString(
            Config::default_hotkeys[selected.row()].shortcut.first, QKeySequence::NativeText);
        const bool key_sequence_used = IsUsedKey(default_key_sequence);

        if (key_sequence_used &&
            default_key_sequence != QKeySequence(model->data(selected).toString())) {

            QMessageBox::warning(
                this, tr("Conflicting Key Sequence"),
                tr("The default key sequence is already assigned to another key."));
        } else {
            model->setData(selected, default_key_sequence.toString(QKeySequence::NativeText));
        }
    });
    connect(clear, &QAction::triggered, [this, selected] { model->setData(selected, tr("")); });

    context_menu.exec(ui->hotkey_list->viewport()->mapToGlobal(menu_location));
}

void ConfigureHotkeys::RetranslateUI() {
    ui->retranslateUi(this);
}
