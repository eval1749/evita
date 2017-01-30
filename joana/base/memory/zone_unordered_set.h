// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_MEMORY_ZONE_UNORDERED_SET_H_
#define JOANA_BASE_MEMORY_ZONE_UNORDERED_SET_H_

#include <functional>
#include <unordered_set>

#include "joana/base/memory/zone.h"
#include "joana/base/memory/zone_allocator.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// ZoneUnorderedSet
// A wrapper subclass for |std::unordered_set|.
//
template <typename T,
          typename Hash = std::hash<T>,
          typename KeyEqual = std::equal_to<T>>
class ZoneUnorderedSet
    : public std::unordered_set<T, Hash, KeyEqual, ZoneAllocator<T>> {
 public:
  explicit ZoneUnorderedSet(Zone* zone) : BaseClass(ZoneAllocator<T>(zone)) {}

  template <typename InputIt>
  ZoneUnorderedSet(Zone* zone, InputIt first, InputIt last)
      : BaseClass(first, last, 0, Hash(), KeyEqual(), ZoneAllocator<T>(zone)) {}
};

}  // namespace joana

#endif  // JOANA_BASE_MEMORY_ZONE_UNORDERED_SET_H_
