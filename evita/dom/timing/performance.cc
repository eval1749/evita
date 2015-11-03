// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/performance.h"

#include "base/time/time.h"

namespace dom {

Performance::Performance() {}
Performance::~Performance() {}

double Performance::Now() const {
  return base::Time::Now().ToDoubleT();
}

}  // namespace dom
