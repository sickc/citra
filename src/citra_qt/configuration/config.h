﻿// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <QVariant>
#include "citra_qt/uisettings.h"
#include "core/settings.h"

class QSettings;

class Config {
public:
    Config();
    ~Config();

    void Reload();
    void Save();

    static const std::array<int, Settings::NativeButton::NumButtons> default_buttons;
    static const std::array<std::array<int, 5>, Settings::NativeAnalog::NumAnalogs> default_analogs;
    static const std::array<UISettings::Shortcut, 25> default_hotkeys;

private:
    void ReadValues();
    void ReadAudioValues();
    void ReadCameraValues();
    void ReadControlValues();
    void ReadCoreValues();
    void ReadDataStorageValues();
    void ReadDebuggingValues();
    void ReadLayoutValues();
    void ReadMiscellaneousValues();
    void ReadMultiplayerValues();
    void ReadPathValues();
    void ReadCpuValues();
    void ReadRendererValues();
    void ReadShortcutValues();
    void ReadSystemValues();
    void ReadUIValues();
    void ReadUIGameListValues();
    void ReadUILayoutValues();
    void ReadUpdaterValues();
    void ReadUtilityValues();
    void ReadWebServiceValues();
    void ReadVideoDumpingValues();

    void SaveValues();
    void SaveAudioValues();
    void SaveCameraValues();
    void SaveControlValues();
    void SaveCoreValues();
    void SaveDataStorageValues();
    void SaveDebuggingValues();
    void SaveLayoutValues();
    void SaveMiscellaneousValues();
    void SaveMultiplayerValues();
    void SavePathValues();
    void SaveCpuValues();
    void SaveRendererValues();
    void SaveShortcutValues();
    void SaveSystemValues();
    void SaveUIValues();
    void SaveUIGameListValues();
    void SaveUILayoutValues();
    void SaveUpdaterValues();
    void SaveUtilityValues();
    void SaveWebServiceValues();
    void SaveVideoDumpingValues();

    QVariant ReadSetting(const QString& name) const;
    QVariant ReadSetting(const QString& name, const QVariant& default_value) const;
    void WriteSetting(const QString& name, const QVariant& value);
    void WriteSetting(const QString& name, const QVariant& value, const QVariant& default_value);

    std::unique_ptr<QSettings> qt_config;
    std::string qt_config_loc;
};
