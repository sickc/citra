// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include <memory>

#include "common/common_types.h"

namespace Memory {
class PageTable;
}

namespace Common {

class FastmemMapper;

class BackingMemory final {
public:
    ~BackingMemory();

    u8* Get() const {
        return pointer;
    }

private:
    friend class FastmemMapper;

    BackingMemory(FastmemMapper* m, u8* p);

    FastmemMapper* mapper;
    u8* pointer;
};

class FastmemRegion final {
public:
    FastmemRegion();
    ~FastmemRegion();

    u8* Get() const {
        return pointer;
    }

private:
    friend class FastmemMapper;

    FastmemRegion(FastmemMapper* m, u8* p);

    FastmemMapper* mapper;
    u8* pointer;
};

class FastmemMapper final {
public:
    /// @param shmem_required Maximum total amount of shared memory that will be `Allocate`-d.
    explicit FastmemMapper(std::size_t shmem_required);
    ~FastmemMapper();

    void* GetBaseAddress() const;
    BackingMemory Allocate(std::size_t size);

    FastmemRegion AllocateFastmemRegion();
    void Map(Memory::PageTable& page_table, VAddr vaddr, u8* backing_memory, std::size_t size);
    void Unmap(Memory::PageTable& page_table, VAddr vaddr, std::size_t size);

private:
    friend class BackingMemory;
    friend class FastmemRegion;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Common
