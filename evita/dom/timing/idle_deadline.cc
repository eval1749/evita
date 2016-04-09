// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/idle_deadline.h"

#include "base/time/time.h"
#include "evita/dom/scheduler/scheduler.h"
#include "evita/dom/script_host.h"

namespace dom {

IdleDeadline::IdleDeadline(const base::TimeTicks& deadline)
    : deadline_(deadline) {}

IdleDeadline::~IdleDeadline() {}

bool IdleDeadline::did_timeout() const {
  return TimeRemaining() <= 0;
}

double IdleDeadline::TimeRemaining() const {
  const auto& now = ScriptHost::instance()->scheduler()->NowTicks();
  return (deadline_ - now).InMillisecondsF();
}

}  // namespace dom
