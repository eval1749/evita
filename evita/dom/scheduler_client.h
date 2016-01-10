// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_CLIENT_H_
#define EVITA_DOM_SCHEDULER_CLIENT_H_

#include "base/macros.h"

namespace base {
class Time;
}

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// SchedulerClient
//
// TODO(eval1749): We should rename |SchedulerClient| to |SchedulerObsever|.
class SchedulerClient {
 public:
  virtual ~SchedulerClient();

  virtual void DidCancelAnimationFrame() = 0;
  virtual void DidUpdateDom() = 0;
  virtual void DidRequestAnimationFrame() = 0;

 protected:
  SchedulerClient();

 private:
  DISALLOW_COPY_AND_ASSIGN(SchedulerClient);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_CLIENT_H_
