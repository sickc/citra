// Copyright 2020 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <cstdlib>
#include <memory>
#include "common/assert.h"
#include "core/backing_memory_manager.h"

namespace Memory {

struct BackingMemoryManager::Impl final {
    u8* memory;
    std::size_t alloc_offset = 0;
    std::size_t max_alloc = 0;
};

BackingMemory::BackingMemory(std::shared_ptr<BackingMemoryManager> manager_, u8* pointer_, MemoryRef ref_, size_t size_) : manager(manager_), pointer(pointer_), ref(ref_), size(size_) {}
BackingMemory::~BackingMemory() = default;

FastmemRegion::FastmemRegion() : manager(nullptr), pointer(nullptr) {}
FastmemRegion::FastmemRegion(std::shared_ptr<BackingMemoryManager> manager_, u8* pointer_) : manager(manager_), pointer(pointer_) {}
FastmemRegion::~FastmemRegion() = default;

BackingMemoryManager::BackingMemoryManager(std::size_t total_required) : impl(std::make_unique<Impl>()) {
    impl->max_alloc = total_required;
    impl->memory = static_cast<u8*>(std::malloc(total_required));
}

BackingMemoryManager::~BackingMemoryManager() {
    std::free(static_cast<void*>(impl->memory));
}

BackingMemory BackingMemoryManager::AllocateBackingMemory(std::size_t size) {
    size_t current_offset = impl->alloc_offset;
    impl->alloc_offset += size;
    ASSERT(impl->alloc_offset <= impl->max_alloc);

    return BackingMemory{shared_from_this(), impl->memory + current_offset, static_cast<MemoryRef>(current_offset), size};
}

u8* BackingMemoryManager::GetPointerForRef(MemoryRef ref) {
    return impl->memory + ref;
}

MemoryRef BackingMemoryManager::GetRefForPointer(u8* pointer) {
    return MemoryRef{pointer - impl->memory};
}

FastmemRegion BackingMemoryManager::AllocateFastmemRegion() {
    return {};
}

void BackingMemoryManager::Map(Memory::PageTable&, VAddr, u8* in, std::size_t) {
    const std::ptrdiff_t offset = in - impl->memory;
    ASSERT(0 <= offset && offset < static_cast<std::ptrdiff_t>(impl->max_alloc));
}

void BackingMemoryManager::Unmap(Memory::PageTable&, VAddr, std::size_t) {}

void BackingMemoryManager::Serialize(std::array<std::ptrdiff_t, PAGE_TABLE_NUM_ENTRIES>& out, const std::array<u8*, PAGE_TABLE_NUM_ENTRIES>& in) {
    for (size_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; ++i) {
        if (in[i] == nullptr) {
            out[i] = -1;
        } else {
            const std::ptrdiff_t offset = in[i] - impl->memory;
            ASSERT(0 <= offset && offset < static_cast<std::ptrdiff_t>(impl->max_alloc));
            out[i] = offset;
        }
    }
}

void BackingMemoryManager::Unserialize(std::array<u8*, PAGE_TABLE_NUM_ENTRIES>& out, const std::array<std::ptrdiff_t, PAGE_TABLE_NUM_ENTRIES>& in) {
    for (size_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; ++i) {
        if (in[i] == -1) {
            out[i] = nullptr;
        } else {
            const std::ptrdiff_t offset = in[i];
            ASSERT(0 <= offset && offset < static_cast<std::ptrdiff_t>(impl->max_alloc));
            out[i] = impl->memory + offset;
        }
    }
}

} // namespace Memory
