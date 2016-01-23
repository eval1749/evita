// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/base/memory/zone_owner.h"

namespace evita {

ZoneOwner::ZoneOwner(ZoneOwner&& other) : zone_(std::move(other.zone_)) {}

ZoneOwner::ZoneOwner(const char* name) : zone_(name) {}

ZoneOwner::~ZoneOwner() {}

ZoneOwner& ZoneOwner::operator=(ZoneOwner&& other) {
  zone_ = std::move(other.zone_);
  return *this;
}

void* ZoneOwner::Allocate(size_t size) {
  return zone()->Allocate(size);
}

}  // namespace evita
