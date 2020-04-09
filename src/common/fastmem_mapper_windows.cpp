// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include <memoryapi.h>
#include "common/assert.h"
#include "common/common_types.h"
#include "common/fastmem_mapper.h"
#include "core/memory.h"

namespace Common {

constexpr std::size_t fastmem_region_size = 0x100000000; // 4 GiB
constexpr std::size_t alloc_size = 0x10000;              // 64 KiB
constexpr std::size_t alloc_mask = alloc_size - 1;

namespace {
struct Allocation {
    u8* region_start;
    u8* region_end;
    std::size_t alloc_offset;
};

static BOOL (*UnmapViewOfFile2)(HANDLE Process, PVOID BaseAddress, ULONG UnmapFlags);
static PVOID (*VirtualAlloc2)(HANDLE Process, PVOID BaseAddress, SIZE_T Size, ULONG AllocationType,
                              ULONG PageProtection, MEM_EXTENDED_PARAMETER* ExtendedParameters,
                              ULONG ParameterCount);
static PVOID (*MapViewOfFile3)(HANDLE FileMapping, HANDLE Process, PVOID BaseAddress,
                               ULONG64 Offset, SIZE_T ViewSize, ULONG AllocationType,
                               ULONG PageProtection, MEM_EXTENDED_PARAMETER* ExtendedParameters,
                               ULONG ParameterCount);
static bool fastmem_enabled = false;

struct LoadFunctions {
    LoadFunctions() {
        HMODULE h = ::LoadLibraryW(L"kernelbase.dll");
        ASSERT(h);
        UnmapViewOfFile2 = (decltype(UnmapViewOfFile2))::GetProcAddress(h, "UnmapViewOfFile2");
        VirtualAlloc2 = (decltype(VirtualAlloc2))::GetProcAddress(h, "VirtualAlloc2");
        MapViewOfFile3 = (decltype(MapViewOfFile3))::GetProcAddress(h, "MapViewOfFile3");
        fastmem_enabled = UnmapViewOfFile2 && VirtualAlloc2 && MapViewOfFile3;
    }
} load_functions;

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

} // anonymous namespace

struct FastmemMapper::Impl {
    HANDLE shmem;
    std::size_t alloc_offset = 0;
    std::size_t max_alloc = 0;

    std::vector<Allocation> allocations;

    auto FindAllocation(u8* backing_memory) const {
        return std::find_if(
            allocations.begin(), allocations.end(), [backing_memory](const auto& x) {
                return backing_memory >= x.region_start && backing_memory < x.region_end;
            });
    }

    bool IsMappable(Memory::PageTable& page_table, VAddr vaddr);
    bool IsMapped(Memory::PageTable& page_table, VAddr vaddr);
    void DoMap(Memory::PageTable& page_table, VAddr vaddr,
               std::vector<Allocation>::const_iterator allocation);
    void DoUnmap(Memory::PageTable& page_table, VAddr vaddr);
};

BackingMemory::BackingMemory(FastmemMapper* m, u8* p) : mapper(m), pointer(p) {}

BackingMemory::~BackingMemory() {
    UnmapViewOfFile(pointer);

    const auto allocation = mapper->impl->FindAllocation(pointer);
    ASSERT(allocation != mapper->impl->allocations.end());
    mapper->impl->allocations.erase(allocation);
}

FastmemRegion::FastmemRegion() : FastmemRegion(nullptr, nullptr) {}
FastmemRegion::FastmemRegion(FastmemMapper* m, u8* p) : mapper(m), pointer(p) {}

FastmemRegion::~FastmemRegion() {
    VirtualFree(pointer, fastmem_region_size, MEM_RELEASE);
}

FastmemMapper::FastmemMapper(std::size_t shmem_required) : impl(std::make_unique<Impl>()) {
    impl->max_alloc = shmem_required;

    impl->shmem = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                     static_cast<DWORD>(shmem_required), nullptr);
    if (!impl->shmem) {
        LOG_WARNING(Common_Memory, "Unable to fastmem: CreateFileMappingA failed");
        return;
    }
}

FastmemMapper::~FastmemMapper() {
    if (impl->shmem) {
        CloseHandle(impl->shmem);
    }
}

BackingMemory FastmemMapper::Allocate(std::size_t size) {
    size_t current_offset = impl->alloc_offset;
    impl->alloc_offset += size;
    ASSERT(impl->alloc_offset <= impl->max_alloc);

    u8* region_start = static_cast<u8*>(MapViewOfFile(impl->shmem, FILE_MAP_ALL_ACCESS,
                                                      static_cast<DWORD>(current_offset >> 32),
                                                      static_cast<DWORD>(current_offset), size));
    ASSERT(region_start);
    impl->allocations.emplace_back(Allocation{region_start, region_start + size, current_offset});
    return BackingMemory(this, region_start);
}

FastmemRegion FastmemMapper::AllocateFastmemRegion() {
    if (!fastmem_enabled) {
        ASSERT_MSG(false, "Fastmem not enabled (Unsupported version of Windows)");
        return {};
    }

    void* base = (*VirtualAlloc2)(GetCurrentProcess(), nullptr, fastmem_region_size,
                                  MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
    if (!base) {
        ASSERT_MSG(false, "Unable to fastmem: could not mmap fastmem region");
        return {};
    }

    // NOTE: Terminate one alloc_size before the end. This is because when splitting a placeholder,
    // Windows requires the remaining size to be nonzero.
    for (size_t i = 0; i < fastmem_region_size - alloc_size; i += alloc_size) {
        if (!VirtualFree((u8*)base + i, alloc_size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER)) {
            ASSERT_MSG(false, "Could not VirtualFree: {} {}", i, GetLastErrorAsString());
        }
    }

    return FastmemRegion(this, static_cast<u8*>(base));
}

bool FastmemMapper::Impl::IsMappable(Memory::PageTable& page_table, VAddr vaddr) {
    const VAddr v = vaddr & ~alloc_mask;
    u8* const base_ptr = page_table.pointers[v >> Memory::PAGE_BITS];

    for (size_t i = 0; i < alloc_size; i += Memory::PAGE_SIZE)
        if (page_table.pointers[(v + i) >> Memory::PAGE_BITS] != base_ptr + i)
            return false;

    return FindAllocation(base_ptr) != allocations.end();
}

bool FastmemMapper::Impl::IsMapped(Memory::PageTable& page_table, VAddr vaddr) {
    const VAddr v = vaddr & ~alloc_mask;
    MEMORY_BASIC_INFORMATION info;
    size_t result = VirtualQuery(page_table.fastmem_base.Get() + v, &info, sizeof(info));
    ASSERT(result);
    return info.Type == MEM_MAPPED;
}

void FastmemMapper::Impl::DoMap(Memory::PageTable& page_table, VAddr vaddr,
                                std::vector<Allocation>::const_iterator allocation) {
    const VAddr v = vaddr & ~alloc_mask;
    u8* const base_ptr = page_table.pointers[v >> Memory::PAGE_BITS];
    const std::size_t offset = base_ptr - allocation->region_start;
    const bool success =
        (*MapViewOfFile3)(shmem, nullptr, page_table.fastmem_base.Get() + v, offset, alloc_size,
                          MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);
    if (!success) {
        ASSERT_MSG(false, "Could not MapViewOfFile3: {}", GetLastErrorAsString());
    }
}

void FastmemMapper::Impl::DoUnmap(Memory::PageTable& page_table, VAddr vaddr) {
    const VAddr v = vaddr & ~alloc_mask;
    const bool success = (*UnmapViewOfFile2)(GetCurrentProcess(), page_table.fastmem_base.Get() + v,
                                             MEM_PRESERVE_PLACEHOLDER);
    if (!success)
        ASSERT_MSG(false, "Could not UnmapViewOfFile2: {}", GetLastErrorAsString());
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

    for (std::size_t vaddr_offset = 0; vaddr_offset < size; vaddr_offset += alloc_size) {
        const VAddr v = vaddr + vaddr_offset;
        const bool mappable = impl->IsMappable(page_table, v);
        const bool mapped = impl->IsMapped(page_table, v);

        if (mapped) {
            impl->DoUnmap(page_table, v);
        }

        if (mappable) {
            impl->DoMap(page_table, v, allocation);
        }
    }
}

void FastmemMapper::Unmap(Memory::PageTable& page_table, VAddr vaddr, std::size_t size) {
    if (!page_table.fastmem_base.Get()) {
        return;
    }

    for (std::size_t vaddr_offset = 0; vaddr_offset < size; vaddr_offset += alloc_size) {
        const VAddr v = vaddr + vaddr_offset;
        const bool mapped = impl->IsMapped(page_table, v);

        if (mapped) {
            impl->DoUnmap(page_table, v);
        }
    }
}

} // namespace Common
