// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ZONE_DEQUE_H_
#define EVITA_BASE_MEMORY_ZONE_DEQUE_H_

#include <deque>

#include "evita/base/memory/zone.h"
#include "evita/base/memory/zone_allocator.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// ZoneDeque
// A wrapper subclass for |std::deque|.
//
template <typename T>
class ZoneDeque : public std::deque<T, ZoneAllocator<T>> {
 public:
  explicit ZoneDeque(Zone* zone)
      : std::deque<T, ZoneAllocator<T>>(ZoneAllocator<T>(zone)) {}

  ZoneDeque(Zone* zone, size_t size, const value_type& val = value_type())
      : std::deque<T, ZoneAllocator<T>>(size, val, ZoneAllocator<T>(zone)) {}

  ZoneDeque(Zone* zone, const std::deque<T>& other)
      : std::deque<T, ZoneAllocator<T>>(other.begin(),
                                        other.end(),
                                        ZoneAllocator<T>(zone)) {}
};

}  // namespace evita

#endif  // EVITA_BASE_MEMORY_ZONE_DEQUE_H_
