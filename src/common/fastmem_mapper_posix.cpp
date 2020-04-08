// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "common/assert.h"
#include "common/common_types.h"
#include "common/fastmem_mapper.h"
#include "core/memory.h"

namespace Common {

constexpr std::size_t fastmem_region_size = 0x100000000; // 4 GiB

namespace {
struct Allocation {
    u8* region_start;
    u8* region_end;
    std::size_t alloc_offset;
};
} // anonymous namespace

struct FastmemMapper::Impl {
    int fd = -1;
    std::size_t alloc_offset = 0;
    std::size_t max_alloc = 0;

    std::vector<Allocation> allocations;

    auto FindAllocation(u8* backing_memory) {
        return std::find_if(
            allocations.begin(), allocations.end(), [backing_memory](const auto& x) {
                return backing_memory >= x.region_start && backing_memory < x.region_end;
            });
    }
};

BackingMemory::BackingMemory(FastmemMapper* m, u8* p) : mapper(m), pointer(p) {}

BackingMemory::~BackingMemory() {
    const auto allocation = mapper->impl->FindAllocation(pointer);
    ASSERT(allocation != mapper->impl->allocations.end());

    const std::size_t size = allocation->region_end - allocation->region_start;
    munmap(pointer, size);
    mapper->impl->allocations.erase(allocation);
}

FastmemRegion::FastmemRegion() : mapper(nullptr), pointer(nullptr) {}
FastmemRegion::FastmemRegion(FastmemMapper* m, u8* p) : mapper(m), pointer(p) {}

FastmemRegion::~FastmemRegion() {
    if (pointer) {
        munmap(pointer, fastmem_region_size);
    }
}

FastmemMapper::FastmemMapper(std::size_t shmem_required) : impl(std::make_unique<Impl>()) {
    impl->max_alloc = shmem_required;

    const std::string shm_filename = "/citra." + std::to_string(getpid());
    impl->fd = shm_open(shm_filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    if (impl->fd == -1) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: shm_open failed");
        return;
    }
    shm_unlink(shm_filename.c_str());
    if (ftruncate(impl->fd, shmem_required) < 0) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: could not allocate shared memory");
        return;
    }
}

FastmemMapper::~FastmemMapper() {
    if (impl->fd != -1) {
        close(impl->fd);
    }
}

BackingMemory FastmemMapper::Allocate(std::size_t size) {
    size_t current_offset = impl->alloc_offset;
    impl->alloc_offset += size;
    ASSERT(impl->alloc_offset <= impl->max_alloc);

    u8* region_start = static_cast<u8*>(
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, impl->fd, current_offset));
    ASSERT(region_start && region_start != MAP_FAILED);
    impl->allocations.emplace_back(Allocation{region_start, region_start + size, current_offset});
    return BackingMemory(this, region_start);
}

FastmemRegion FastmemMapper::AllocateFastmemRegion() {
    void* base = mmap(nullptr, fastmem_region_size, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (base == MAP_FAILED) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: could not mmap fastmem region");
        return FastmemRegion(this, nullptr);
    }
    return FastmemRegion(this, static_cast<u8*>(base));
}

void FastmemMapper::Map(Memory::PageTable& page_table, VAddr vaddr, u8* backing_memory,
                        std::size_t size) {
    if (!page_table.fastmem_base.Get()) {
        return;
    }

    const auto allocation = impl->FindAllocation(backing_memory);

    if (allocation == impl->allocations.end()) {
        Unmap(page_table, vaddr, size);
        return;
    }

    const std::size_t offset = allocation->alloc_offset +
                               static_cast<std::size_t>(backing_memory - allocation->region_start);
    size = std::min(size, static_cast<std::size_t>(allocation->region_end - backing_memory));
    if (!size) {
        return;
    }

    void* result = mmap(page_table.fastmem_base.Get() + vaddr, size, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_FIXED, impl->fd, offset);
    DEBUG_ASSERT(result != MAP_FAILED);
}

void FastmemMapper::Unmap(Memory::PageTable& page_table, VAddr vaddr, std::size_t size) {
    if (!page_table.fastmem_base.Get()) {
        return;
    }

    void* result = mmap(page_table.fastmem_base.Get() + vaddr, size, PROT_NONE,
                        MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0);
    DEBUG_ASSERT(result != MAP_FAILED);
}

} // namespace Common
