// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <atomic>

#include "evita/base/ping_provider.h"

#include "base/logging.h"

namespace base {

//////////////////////////////////////////////////////////////////////
//
// PingProvider
//
PingProvider::PingProvider() {}
PingProvider::~PingProvider() {}

void PingProvider::PingInternal(std::atomic<bool>* ping_to_false) {
  DCHECK(ping_to_false->load());
  ping_to_false->store(false);
}

}  // namespace base
