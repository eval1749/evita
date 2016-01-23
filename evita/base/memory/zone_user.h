// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ZONE_USER_H_
#define EVITA_BASE_MEMORY_ZONE_USER_H_

#include "base/macros.h"
#include "evita/base/evita_base_export.h"

namespace evita {

class Zone;

//////////////////////////////////////////////////////////////////////
//
// ZoneUser
//
class EVITA_BASE_EXPORT ZoneUser {
 public:
  ~ZoneUser();

  Zone* zone() const { return zone_; }

 protected:
  explicit ZoneUser(Zone* zone);

 private:
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(ZoneUser);
};

}  // namespace evita

#endif  // EVITA_BASE_MEMORY_ZONE_USER_H_
