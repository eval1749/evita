// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_REPEAT_CONTROLLER_H_
#define EVITA_UI_BASE_REPEAT_CONTROLLER_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/timer/timer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// RepeatController
//
//  An object that handles auto-repeating UI actions. There is a longer initial
//  delay after which point repeats become constant. Users provide a callback
//  that is notified when each repeat occurs so that they can perform the
//  associated action.
//
class RepeatController final {
 public:
  explicit RepeatController(const base::Closure& callbacK);
  ~RepeatController();

  void Start();
  void Stop();

 private:
  void DidFireTimer();

  const base::Closure callback_;
  base::OneShotTimer timer_;

  DISALLOW_COPY_AND_ASSIGN(RepeatController);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_REPEAT_CONTROLLER_H_
