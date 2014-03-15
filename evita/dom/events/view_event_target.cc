// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_target.h"

#include "evita/dom/events/view_event_target_set.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// ViewEventTargetClass
//
class ViewEventTargetClass
    : public v8_glue::DerivedWrapperInfo<ViewEventTarget, EventTarget> {

  public: ViewEventTargetClass(const char* name)
      : BaseClass(name) {
  }
  public: ~ViewEventTargetClass() = default;

  DISALLOW_COPY_AND_ASSIGN(ViewEventTargetClass);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ViewEventTarget
//
DEFINE_SCRIPTABLE_OBJECT(ViewEventTarget, ViewEventTargetClass);

ViewEventTarget::ViewEventTarget()
    : event_target_id_(ViewEventTargetSet::instance()->Register(this)) {
}

ViewEventTarget::~ViewEventTarget() {
}

}  // namespace dom
