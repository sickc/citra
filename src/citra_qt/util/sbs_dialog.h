// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QMessageBox>
#include <QWidget>

/// move dialog to centre of left eye in SBS 3D mode
void MoveDialogToLeftEye(QWidget* dialog, QWidget* parent, int offset = 0);

// move dialog to centre of right eye in SBS 3D mode
void MoveDialogToRightEye(QWidget* dialog, QWidget* parent, int offset = 0);

class MessageBox3D : public QMessageBox {
    Q_OBJECT
public:
    static StandardButton information(QWidget* parent, const QString& title, const QString& text,
                                      StandardButtons buttons = Ok,
                                      StandardButton defaultButton = NoButton);
    static StandardButton question(QWidget* parent, const QString& title, const QString& text,
                                   StandardButtons buttons = StandardButtons(Yes | No),
                                   StandardButton defaultButton = NoButton);
    static StandardButton warning(QWidget* parent, const QString& title, const QString& text,
                                  StandardButtons buttons = Ok,
                                  StandardButton defaultButton = NoButton);
    static StandardButton critical(QWidget* parent, const QString& title, const QString& text,
                                   StandardButtons buttons = Ok,
                                   StandardButton defaultButton = NoButton);
    static MessageBox3D::StandardButton showNewMessageBox(
        QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
        QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);
    int exec() override;
};
