// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_view_event_target_h)
#define INCLUDE_evita_dom_events_view_event_target_h

#include "evita/dom/events/event_target.h"

namespace dom {

class ViewEventTargetSet;

namespace bindings {
class ViewEventTargetClass;
}

class ViewEventTarget
    : public v8_glue::Scriptable<ViewEventTarget, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(ViewEventTarget);
  friend class bindings::ViewEventTargetClass;

  friend class ViewEventTargetSet;

  private: domapi::EventTargetId event_target_id_;

  protected: ViewEventTarget();
  protected: virtual ~ViewEventTarget();

  public: domapi::EventTargetId event_target_id() const {
      return event_target_id_;
  }
  public: domapi::EventTargetId window_id() const { return event_target_id(); }

  protected: void ReleaseCapture();
  protected: void SetCapture();

  DISALLOW_COPY_AND_ASSIGN(ViewEventTarget);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_view_event_target_h)
