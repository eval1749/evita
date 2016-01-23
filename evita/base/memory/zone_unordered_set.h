// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ZONE_UNORDERED_SET_H_
#define EVITA_BASE_MEMORY_ZONE_UNORDERED_SET_H_

#include <unordered_set>

#include "evita/base/memory/zone.h"
#include "evita/base/memory/zone_allocator.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedSet
// A wrapper subclass for |std::unordered_set|.
//
template <typename T>
class ZoneUnorderedSet
    : public std::unordered_set<T,
                                typename std::unordered_set<T>::hasher,
                                typename std::unordered_set<T>::key_equal,
                                ZoneAllocator<T>> {
  typedef std::unordered_set<T,
                             typename std::unordered_set<T>::hasher,
                             typename std::unordered_set<T>::key_equal,
                             ZoneAllocator<T>> BaseClass;

 public:
  explicit ZoneUnorderedSet(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}
};

}  // namespace evita

#endif  // EVITA_BASE_MEMORY_ZONE_UNORDERED_SET_H_
