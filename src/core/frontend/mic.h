// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include "common/swap.h"

namespace Frontend {

namespace Mic {

enum class Signedness : u8 {
    Signed,
    Unsigned,
};

struct Parameters {
    Signedness sign;
    u8 sample_size;
    bool buffer_loop;
    u32 sample_rate;
    u32 buffer_offset;
    u32 buffer_size;
};

class Interface {
public:
    /// Starts the microphone. Called by Core
    virtual void StartRecording(Parameters params) = 0;

    /// Stops the microphone. Called by Core
    virtual void StopRecording() = 0;

    /// Sets the backing memory that the mic should write raw samples into. Called by Core
    void SetBackingMemory(u8* pointer, u32 size) {
        backing_memory = pointer;
        backing_memory_size = size;
    }

    /// Adjusts the Parameters. Implementations should update the parameters field in addition to
    /// changing the mic to sample according to the new parameters. Called by Core
    virtual void AdjustParameters(Parameters parameters) = 0;

    Parameters GetParameters() const {
        return parameters;
    }

protected:
    u8* backing_memory;
    u32 backing_memory_size;

    Parameters parameters;
};

class DefaultMic final : public Interface {
public:
    void StartRecording(Parameters params) {
        parameters = params;
    }

    void StopRecording() {}

    void AdjustParameters(Parameters params) {
        parameters = params;
    }
};

} // namespace Mic

void RegisterMic(std::shared_ptr<Mic::Interface> mic);

std::shared_ptr<Mic::Interface> GetCurrentMic();

} // namespace Frontend
