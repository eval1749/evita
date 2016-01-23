// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/memory/zone_user.h"

namespace evita {

ZoneUser::ZoneUser(Zone* zone) : zone_(zone) {}

ZoneUser::~ZoneUser() {}

}  // namespace evita
