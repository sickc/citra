// Copyright 2020 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include <boost/serialization/strong_typedef.hpp>

namespace Memory {

BOOST_STRONG_TYPEDEF(std::ptrdiff_t, MemoryRef);

} // namespace Memory

namespace boost::serialization {

template<class Archive>
inline void serialize(Archive& ar, Memory::MemoryRef& ref, const unsigned) {
    ar& ref.t;
}

} // namespace boost::serialization
