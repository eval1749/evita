// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_MEMORY_ZONE_OWNER_H_
#define JOANA_BASE_MEMORY_ZONE_OWNER_H_

#include "base/macros.h"
#include "joana/base/memory/zone.h"
#include "joana/base/base_export.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// ZoneOwner
//
class JOANA_BASE_EXPORT ZoneOwner {
 public:
  ZoneOwner& operator=(ZoneOwner&& other);

  Zone* zone() { return &zone_; }

  // Allocate |size| bytes of memory in the Zone.
  void* Allocate(size_t size);

 protected:
  explicit ZoneOwner(const char* name);
  ZoneOwner(ZoneOwner&& other);
  ~ZoneOwner();

 private:
  Zone zone_;

  DISALLOW_COPY_AND_ASSIGN(ZoneOwner);
};

}  // namespace joana

#endif  // JOANA_BASE_MEMORY_ZONE_OWNER_H_
