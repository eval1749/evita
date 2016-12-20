// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/base/memory/zone_allocated.h"

#include "base/logging.h"
#include "joana/base/memory/zone.h"

namespace joana {

ZoneAllocated::ZoneAllocated() {}

ZoneAllocated::~ZoneAllocated() {
  NOTREACHED();
}

#if 0
void ZoneAllocated::operator delete(void* pointer, Zone* zone) {
  __assume(pointer);
  __assume(zone);
  NOTREACHED();
}
#endif

void* ZoneAllocated::operator new(size_t size, Zone* zone) {
  return zone->Allocate(size);
}

}  // namespace joana
