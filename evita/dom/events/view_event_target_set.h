// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_events_view_event_target_set_h)
#define INCLUDE_evita_dom_events_view_event_target_set_h

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
class ViewEventTargetSet : public common::Singleton<ViewEventTargetSet> {
  DECLARE_SINGLETON_CLASS(ViewEventTargetSet);

  private: std::unordered_map<domapi::EventTargetId,
                              gc::WeakPtr<ViewEventTarget>> map_;
  private: domapi::EventTargetId next_event_target_id_;

  private: ViewEventTargetSet();
  public: ~ViewEventTargetSet();

  public: void DidDestroyWidget(domapi::EventTargetId event_target_id);
  public: ViewEventTarget* Find(domapi::EventTargetId event_target_id) const;
  public: domapi::EventTargetId Register(ViewEventTarget* event_target);
  public: void ResetForTesting();
  public: void Unregister(domapi::EventTargetId event_target_id);

  DISALLOW_COPY_AND_ASSIGN(ViewEventTargetSet);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_view_event_target_set_h)
