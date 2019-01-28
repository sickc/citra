// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <chrono>
#include <thread>
#include "common/logging/log.h"
#include "framedump.h"
#include "settings.h"
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <video_core/video_core.h>
#include <windows.h>

namespace Capture {

Framedump* get_instance() {
    static Framedump movie;
    return &movie;
}

std::string GetLastErrorAsString() {
    // Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); // No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

void Framedump::Start(u64 width, u64 height) {
    if (is_open) {
        return;
    }

    auto height_i = (int)(height);
    auto width_i = (int)(width);

    LOG_INFO(Frontend, "Start: {} x {}", width_i, height_i);
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    //Create pipe for upper screen video
    outPipeUpper = CreateNamedPipe(TEXT("\\\\.\\pipe\\citravideoupper"), PIPE_ACCESS_OUTBOUND,
                              PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
                              static_cast<DWORD>(width_i * height_i * 4 * 120), 0, 0, nullptr);
    //TODO: Implement lower screen pipe
    //Create pipe for audio
    outPipeAudio = CreateNamedPipe(TEXT("\\\\.\\pipe\\citraaudio"), PIPE_ACCESS_OUTBOUND,
                                   PIPE_TYPE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
                                   160 * 2 * 4 * 120, 0, 0, nullptr);

    if (outPipeUpper == INVALID_HANDLE_VALUE || outPipeAudio == INVALID_HANDLE_VALUE) {
        LOG_CRITICAL(Frontend, "Failed to open file!");
        return;
    }

    auto height_str = std::to_string(height_i);
    auto width_str = std::to_string(width_i);

    auto string = std::string();
    string += "\"ffmpeg.exe\" -report -r 60 "
              "-f rawvideo -s ";
    string += width_str;
    string += "x";
    string += height_str;
    string += " -pixel_format rgb32 -i \"async:\\\\.\\pipe\\citravideoupper\" "
              "-f s16le -guess_layout_max 0 -ar 32.728k -ac 2 -i \"async:\\\\.\\pipe\\citraaudio\" "
              "-vf vflip -pix_fmt rgb32 -c:v libx264 -crf 0 test.mkv"; //TODO: Make video encoding parameters accessible via some nice GUI menu

    LOG_INFO(Frontend, "CLI args: {}", string.c_str());

    if (!CreateProcess(nullptr, const_cast<LPSTR>(string.c_str()), nullptr, nullptr, TRUE,
                       CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        LOG_CRITICAL(Frontend, "CreateProcess failed ({})", GetLastErrorAsString().c_str());
        return;
    }

    is_open = true;
    LOG_INFO(Frontend, "Start done!");
}

void Framedump::EncodeAudio(const s16* input, u64 sample_count) {
    DWORD written;
    auto success = static_cast<bool>(WriteFile(
        outPipeAudio, (void*)input, static_cast<DWORD>(sample_count * 2), &written, nullptr));
    if (!success) {
        LOG_TRACE(Frontend, "Failed to write to audio pipe: {}", GetLastErrorAsString().c_str());
        return;
    }
    LOG_TRACE(Frontend, "Written %d bytes of audio to ffmpeg.", written);
}

void Framedump::EncodeVideo(void* input, u64 size) {
    DWORD written;
    auto success =
        static_cast<bool>(WriteFile(outPipeUpper, input, static_cast<DWORD>(size), &written, nullptr));
    if (!success) {
        LOG_TRACE(Frontend, "Failed to write to video pipe: {}", GetLastErrorAsString().c_str());
        return;
    }
    LOG_TRACE(Frontend, "Written %d bytes of video to ffmpeg.", written);
}

void Framedump::Stop() {
    LOG_INFO(Frontend, "Stopping framedump!");
    if (!is_open) {
        return;
    }
    VideoCore::g_renderer_framedump_enabled = false;

    //Close pipes
    DisconnectNamedPipe(outPipeUpper);
    DisconnectNamedPipe(outPipeAudio);

    WaitForSingleObject(pi.hProcess, INFINITE);
    LOG_INFO(Frontend, "FFMPEG IS DED!");

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    is_open = false;
}

} // namespace Capture
