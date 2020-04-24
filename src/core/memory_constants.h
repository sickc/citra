// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

namespace Memory {

// Are defined in a system header
#undef PAGE_SIZE
#undef PAGE_MASK
/**
 * Page size used by the ARM architecture. This is the smallest granularity with which memory can
 * be mapped.
 */
const u32 PAGE_SIZE = 0x1000;
const u32 PAGE_MASK = PAGE_SIZE - 1;
const int PAGE_BITS = 12;
const std::size_t PAGE_TABLE_NUM_ENTRIES = 1 << (32 - PAGE_BITS);

} // namespace Memory
