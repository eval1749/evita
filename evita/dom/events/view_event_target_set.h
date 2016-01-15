// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_SET_H_
#define EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_SET_H_

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/public/event_target_id.h"
#include "evita/gc/weak_ptr.h"

namespace dom {

class ViewEventTarget;

//////////////////////////////////////////////////////////////////////
//
// ViewEventTargetSet
//
// This class represents mapping from widget id to DOM EventTarget object.
//
class ViewEventTargetSet final : public common::Singleton<ViewEventTargetSet> {
  DECLARE_SINGLETON_CLASS(ViewEventTargetSet);

 public:
  ~ViewEventTargetSet() final;

  void DidDestroyWindow(domapi::EventTargetId event_target_id);
  ViewEventTarget* Find(domapi::EventTargetId event_target_id) const;
  domapi::EventTargetId Register(ViewEventTarget* event_target);
  void ResetForTesting();
  void Unregister(domapi::EventTargetId event_target_id);

 private:
  ViewEventTargetSet();

  std::unordered_map<domapi::EventTargetId, gc::WeakPtr<ViewEventTarget>> map_;
  domapi::EventTargetId next_event_target_id_;

  DISALLOW_COPY_AND_ASSIGN(ViewEventTargetSet);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_VIEW_EVENT_TARGET_SET_H_
