// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_H_
#define EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_H_

#include <utility>

#include "evita/dom/events/event_target.h"

namespace domapi {
struct FocusEvent;
struct KeyboardEvent;
struct MouseEvent;
}

namespace dom {

class FocusEvent;
class KeyboardEvent;
class MouseEvent;
class ViewEventTargetSet;

namespace bindings {
class ViewEventTargetClass;
}

//////////////////////////////////////////////////////////////////////
//
// ViewEventTargetClass
//
class ViewEventTarget : public ginx::Scriptable<ViewEventTarget, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(ViewEventTarget);

 public:
  ~ViewEventTarget() override;

  domapi::EventTargetId event_target_id() const { return event_target_id_; }
  domapi::EventTargetId window_id() const { return event_target_id(); }

  virtual std::pair<EventTarget*, FocusEvent*> TranslateFocusEvent(
      const domapi::FocusEvent& api_event,
      EventTarget* related_target);

  virtual std::pair<EventTarget*, KeyboardEvent*> TranslateKeyboardEvent(
      const domapi::KeyboardEvent& api_event);

  virtual std::pair<EventTarget*, MouseEvent*> TranslateMouseEvent(
      const domapi::MouseEvent& api_event);

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
