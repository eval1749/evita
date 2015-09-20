// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_H_
#define EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_H_

#include "evita/dom/events/event_target.h"

namespace dom {

class ViewEventTargetSet;

namespace bindings {
class ViewEventTargetClass;
}

class ViewEventTarget
    : public v8_glue::Scriptable<ViewEventTarget, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(ViewEventTarget);

 public:
  ~ViewEventTarget() override;

  domapi::EventTargetId event_target_id() const { return event_target_id_; }
  domapi::EventTargetId window_id() const { return event_target_id(); }

 protected:
  ViewEventTarget();

  void ReleaseCapture();
  void SetCapture();

 private:
  friend class bindings::ViewEventTargetClass;
  friend class ViewEventTargetSet;

  domapi::EventTargetId event_target_id_;

  DISALLOW_COPY_AND_ASSIGN(ViewEventTarget);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_H_
