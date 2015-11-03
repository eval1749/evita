// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_PERFORMANCE_H_
#define EVITA_DOM_TIMING_PERFORMANCE_H_

#include "evita/dom/events/event_target.h"

namespace dom {

namespace bindings {
class PerformanceClass;
}

//////////////////////////////////////////////////////////////////////
//
// Performance
//
// An implementation of Performance interface defined in:
// https://w3c.github.io/hr-time
//
class Performance final : public v8_glue::Scriptable<Performance, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Performance);

 public:
  Performance();
  ~Performance() final;

 private:
  friend class bindings::PerformanceClass;

  double Now() const;

  DISALLOW_COPY_AND_ASSIGN(Performance);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_PERFORMANCE_H_
