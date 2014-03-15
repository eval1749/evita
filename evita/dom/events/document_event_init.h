// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_document_event_init_h)
#define INCLUDE_evita_dom_events_document_event_init_h

#include "evita/dom/events/event_init.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class DocumentEventInit : public EventInit {
  private: gc::Member<Window> view_;

  public: DocumentEventInit();
  public: virtual ~DocumentEventInit();

  public: Window* view() const { return view_.get(); }
  public: void set_view(Window* view) { view_ = view; }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(DocumentEventInit);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_document_event_init_h)
