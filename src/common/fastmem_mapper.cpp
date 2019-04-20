#include <cstdlib>
#include "common/fastmem_mapper.h"

namespace Common {

BackingMemory::BackingMemory(FastmemMapper& m, u8* p) : mapper(m), pointer(p) {}

BackingMemory::~BackingMemory() {
    std::free(static_cast<void*>(p));
}

FastmemRegion::FastmemRegion(FastmemMapper& m, u8* p) : mapper(m), pointer(p) {}

FastmemRegion::~FastmemRegion() {}

struct FastmemMapper::Impl {};

FastmemMapper::FastmemMapper(std::size_t shmem_required) : impl(std::make_unique<Impl>()) {}

FastmemMapper::~FastmemMapper() {}

BackingMemory FastmemMapper::Allocate(std::size_t size) {
    return BackingMemory(*this, static_cast<u8*>(std::malloc(size)));
}

FastmemRegion FastmemMapper::AllocateFastmemRegion() {
    return FastmemRegion(*this, nullptr);
}

void FastmemMapper::Map(Memory::PageTable&, VAddr vaddr, u8* backing_memory, std::size_t size) {}

void FastmemMapper::Unmap(Memory::PageTable&, VAddr vaddr, std::size_t size) {}

} // namespace Common
