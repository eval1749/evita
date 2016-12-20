// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_MEMORY_ZONE_ALLOCATED_H_
#define JOANA_BASE_MEMORY_ZONE_ALLOCATED_H_

#include "base/macros.h"
#include "joana/base/base_export.h"

namespace joana {

class Zone;

//////////////////////////////////////////////////////////////////////
//
// ZoneAllocated
//
class JOANA_BASE_EXPORT ZoneAllocated {
 public:
  void* operator new(size_t size, Zone* zone);

  // |ZoneAllocated| can't have |delete| operator. But MSVC requires them.
  void operator delete(void*, Zone*) = delete;

 protected:
  ZoneAllocated();

  // |ZoneAllocated| can't have destructor operator. But MSVC requires them.
  ~ZoneAllocated();
};

}  // namespace joana

#endif  // JOANA_BASE_MEMORY_ZONE_ALLOCATED_H_
