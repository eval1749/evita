// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_target.h"

#include "evita/dom/events/view_event_target_set.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ViewEventTarget
//
ViewEventTarget::ViewEventTarget()
    : event_target_id_(ViewEventTargetSet::instance()->Register(this)) {}

ViewEventTarget::~ViewEventTarget() {}

void ViewEventTarget::ReleaseCapture() {
  ScriptHost::instance()->view_delegate()->ReleaseCapture(event_target_id());
}

void ViewEventTarget::SetCapture() {
  ScriptHost::instance()->view_delegate()->SetCapture(event_target_id());
}

}  // namespace dom
