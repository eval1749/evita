// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_MEMORY_ZONE_VECTOR_H_
#define JOANA_PUBLIC_MEMORY_ZONE_VECTOR_H_

#include <vector>

#include "joana/public/memory/zone.h"
#include "joana/public/memory/zone_allocator.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// ZoneVector
// A wrapper subclass for |std::vector|.
//
template <typename T>
class ZoneVector : public std::vector<T, ZoneAllocator<T>> {
 public:
  explicit ZoneVector(Zone* zone)
      : std::vector<T, ZoneAllocator<T>>(ZoneAllocator<T>(zone)) {}

  ZoneVector(Zone* zone, size_t size, const value_type& val = value_type())
      : std::vector<T, ZoneAllocator<T>>(size, val, ZoneAllocator<T>(zone)) {}

  ZoneVector(Zone* zone, const std::vector<T>& other)
      : std::vector<T, ZoneAllocator<T>>(other.begin(),
                                         other.end(),
                                         ZoneAllocator<T>(zone)) {}
};

}  // namespace joana

#endif  // JOANA_PUBLIC_MEMORY_ZONE_VECTOR_H_
