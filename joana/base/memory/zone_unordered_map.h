// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_
#define JOANA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_

#include <functional>
#include <unordered_map>

#include "joana/base/memory/zone.h"
#include "joana/base/memory/zone_allocator.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedMap
// A wrapper subclass for |std::unordered_map|.
//
template <typename K,
          typename T,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>>
class ZoneUnorderedMap
    : public std::unordered_map<K, T, Hash, KeyEqual, ZoneAllocator<T>> {
  using BaseClass = std::unordered_map<K, T, Hash, KeyEqual, ZoneAllocator<T>>;

 public:
  explicit ZoneUnorderedMap(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}

  template <typename InputIt>
  ZoneUnorderedMap(Zone* zone, InputIt first, InputIt last)
      : BaseClass(first, last, 0, Hash(), KeyEqual(), ZoneAllocator<T>(zone)) {}
};

}  // namespace joana

#endif  // JOANA_BASE_MEMORY_ZONE_UNORDERED_MAP_H_
