// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstdio>
#include <memory>
#include <common/common_types.h>
#include <core/frontend/emu_window.h>
#include <windows.h>

namespace Capture {

class Framedump {
public:
    void Start(u64 width, u64 height);

    void EncodeAudio(const s16* input, u64 size);

    void EncodeVideo(void* input, u64 size);

    void Stop();

private:
    bool is_open = false;

    void* outPipeUpper = nullptr;
    void* outPipeLower = nullptr;
    void* outPipeAudio = nullptr;

    SECURITY_ATTRIBUTES saAttr{};
    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};
};

Framedump* get_instance();

}; // namespace Capture
