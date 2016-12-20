// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_MEMORY_ZONE_USER_H_
#define JOANA_BASE_MEMORY_ZONE_USER_H_

#include "base/macros.h"
#include "joana/base/base_export.h"

namespace joana {

class Zone;

//////////////////////////////////////////////////////////////////////
//
// ZoneUser
//
class JOANA_BASE_EXPORT ZoneUser {
 public:
  ~ZoneUser();

  Zone* zone() const { return zone_; }

 protected:
  explicit ZoneUser(Zone* zone);

 private:
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(ZoneUser);
};

}  // namespace joana

#endif  // JOANA_BASE_MEMORY_ZONE_USER_H_
