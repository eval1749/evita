// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_IDLE_DEADLINE_PROVIDER_H_
#define EVITA_DOM_TIMING_IDLE_DEADLINE_PROVIDER_H_

#include "base/time/time.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// IdleDeadlineProvider
//
class IdleDeadlineProvider {
 public:
  virtual ~IdleDeadlineProvider();

  virtual base::TimeDelta GetTimeRemaining() const = 0;
  virtual bool IsIdle() const = 0;

 protected:
  IdleDeadlineProvider();

 private:
  DISALLOW_COPY_AND_ASSIGN(IdleDeadlineProvider);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_IDLE_DEADLINE_PROVIDER_H_
