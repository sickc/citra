// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/frontend/mic.h"

namespace AudioCore {

class CubebInput final : public Frontend::Mic::Interface {
public:
    CubebInput();
    ~CubebInput();

    void StartRecording(Frontend::Mic::Parameters params) override;

    void StopRecording() override;

    void AdjustParameters(Frontend::Mic::Parameters params) override {}

private:
    void ResetDevice() {}
    struct Impl;
    std::unique_ptr<Impl> impl;
};

std::vector<std::string> ListCubebInputDevices();

} // namespace AudioCore
