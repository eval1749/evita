// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_target.h"

#include "evita/dom/events/view_event_target_set.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ViewEventTarget
//
ViewEventTarget::ViewEventTarget()
    : event_target_id_(ViewEventTargetSet::instance()->Register(this)) {
}

ViewEventTarget::~ViewEventTarget() {
}

}  // namespace dom
