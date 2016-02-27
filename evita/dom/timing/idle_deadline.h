// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_IDLE_DEADLINE_H_
#define EVITA_DOM_TIMING_IDLE_DEADLINE_H_

#include "evita/ginx/scriptable.h"

#include "base/time/time.h"

namespace dom {

namespace bindings {
class IdleDeadlineClass;
}

//////////////////////////////////////////////////////////////////////
//
// IdleDeadline
//
// An implementation of IdleDeadline interface defined in:
// https://w3c.github.io/requestidlecallback/
//
class IdleDeadline final : public ginx::Scriptable<IdleDeadline> {
  DECLARE_SCRIPTABLE_OBJECT(IdleDeadline);

 public:
  explicit IdleDeadline(const base::TimeTicks& deadline);
  ~IdleDeadline() final;

 private:
  friend class bindings::IdleDeadlineClass;

  // An implementation of IdleDeadline.didTimeout attribute
  bool did_timeout() const;

  // An implementation of IdleDeadline.timeRemaining() method
  double TimeRemaining() const;

  base::TimeTicks deadline_;

  DISALLOW_COPY_AND_ASSIGN(IdleDeadline);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_IDLE_DEADLINE_H_
