// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/idle_deadline.h"

#include "base/time/time.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/timing/idle_deadline_provider.h"

namespace dom {

IdleDeadline::IdleDeadline() {}
IdleDeadline::~IdleDeadline() {}

bool IdleDeadline::did_timeout() const {
  return !ScriptHost::instance()
              ->scheduler()
              ->GetIdleDeadlineProvider()
              ->IsIdle();
}

double IdleDeadline::TimeRemaining() const {
  return ScriptHost::instance()
      ->scheduler()
      ->GetIdleDeadlineProvider()
      ->GetTimeRemaining()
      .InMillisecondsF();
}

}  // namespace dom
