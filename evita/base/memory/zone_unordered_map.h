// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_
#define EVITA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_

#include <unordered_map>

#include "evita/base/memory/zone.h"
#include "evita/base/memory/zone_allocator.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedMap
// A wrapper subclass for |std::unordered_map|.
//
template <typename K, typename T>
class ZoneUnorderedMap
    : public std::unordered_map<K,
                                T,
                                typename std::unordered_map<K, T>::hasher,
                                typename std::unordered_map<K, T>::key_equal,
                                ZoneAllocator<T>> {
  typedef std::unordered_map<K,
                             T,
                             typename std::unordered_map<K, T>::hasher,
                             typename std::unordered_map<K, T>::key_equal,
                             ZoneAllocator<T>> BaseClass;

 public:
  explicit ZoneUnorderedMap(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}
};

}  // namespace evita

#endif  // EVITA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_
