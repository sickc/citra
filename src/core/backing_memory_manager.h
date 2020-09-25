// Copyright 2020 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include <memory>

#include "common/common_types.h"
#include "core/memory_constants.h"
#include "core/memory_ref.h"

namespace Memory {

class BackingMemoryManager;
class PageTable;
class MemorySystem;

class BackingMemory final {
public:
    BackingMemory(std::shared_ptr<BackingMemoryManager> manager_, u8* pointer_, MemoryRef ref_, size_t size_);
    ~BackingMemory();

    u8* Get() const {
        return pointer;
    }

    MemoryRef GetRef() const {
        return ref;
    }

    size_t GetSize() const {
        return size;
    }

private:
    std::shared_ptr<BackingMemoryManager> manager;
    u8* pointer;
    MemoryRef ref;
    size_t size;
};

class FastmemRegion final {
public:
    FastmemRegion();
    FastmemRegion(std::shared_ptr<BackingMemoryManager> manager_, u8* pointer_);
    ~FastmemRegion();

    u8* Get() const {
        return pointer;
    }

private:
    std::shared_ptr<BackingMemoryManager> manager;
    u8* pointer;
};

class BackingMemoryManager final : public std::enable_shared_from_this<BackingMemoryManager> {
public:
    explicit BackingMemoryManager(std::size_t total_required);
    ~BackingMemoryManager();

    /// Allocate backing memory from our pre-allocated chunk of shared memory.
    /// This chunk of memory is automatically freed when BackingMemoryManager is destructed.
    BackingMemory AllocateBackingMemory(std::size_t size);

    /// Allocate a 4GiB chunk of virtual address space for use in a PageTable.
    /// This address space is automatically released when BackingMemoryManager is destructed.
    FastmemRegion AllocateFastmemRegion();

    u8* GetPointerForRef(MemoryRef ref);

    MemoryRef GetRefForPointer(u8* pointer);

private:
    friend class BackingMemory;
    friend class FastmemRegion;
    friend class MemorySystem;
    friend class PageTable;

    void Map(Memory::PageTable& page_table, VAddr vaddr, u8* backing_memory, std::size_t size);
    void Unmap(Memory::PageTable& page_table, VAddr vaddr, std::size_t size);

    void Serialize(std::array<std::ptrdiff_t, PAGE_TABLE_NUM_ENTRIES>& out,
                   const std::array<u8*, PAGE_TABLE_NUM_ENTRIES>& in);
    void Unserialize(std::array<u8*, PAGE_TABLE_NUM_ENTRIES>& out,
                     const std::array<std::ptrdiff_t, PAGE_TABLE_NUM_ENTRIES>& in);

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace Memory
