// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/base/repeat_controller.h"

namespace ui {

// The delay before the first and then subsequent repeats. Values taken from
// XUL code:
// http://mxr.mozilla.org/seamonkey/source/layout/xul/base/src/nsRepeatService.cpp#52
const int kInitialRepeatDelay = 250;
const int kRepeatDelay = 50;

//////////////////////////////////////////////////////////////////////
//
// RepeatController
//
RepeatController::RepeatController(const base::Closure& callbacK)
    : callback_(callbacK) {}

RepeatController::~RepeatController() {}

void RepeatController::Start() {
  timer_.Start(FROM_HERE,
               base::TimeDelta::FromMilliseconds(kInitialRepeatDelay), this,
               &RepeatController::DidFireTimer);
}

void RepeatController::DidFireTimer() {
  timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(kRepeatDelay), this,
               &RepeatController::DidFireTimer);
  callback_.Run();
}

void RepeatController::Stop() {
  timer_.Stop();
}

}  // namespace ui
