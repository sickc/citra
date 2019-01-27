// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QMessageBox>
#include <QWidget>

void MoveDialogToLeftEye(QWidget* dialog, QWidget* parent);

class QMessageBox3D : public QMessageBox {
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
    static QMessageBox3D::StandardButton showNewMessageBox(
        QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
        QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);
    int exec();
};
