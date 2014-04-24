// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_document_event_h)
#define INCLUDE_evita_dom_events_document_event_h

#include "base/strings/string16.h"
#include "evita/dom/events/event.h"
#include "evita/gc/member.h"

namespace dom {

class DocumentEventInit;
class DocumentWindow;

namespace bindings {
class DocumentEventClass;
}

class DocumentEvent : public v8_glue::Scriptable<DocumentEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(DocumentEvent)
  friend class bindings::DocumentEventClass;

  private: gc::Member<DocumentWindow> view_;

  // Expose for|DocumentWindow|.
  public: DocumentEvent(const base::string16& type,
                        const DocumentEventInit& init_dict);
  private: DocumentEvent(const base::string16& type);
  public: virtual ~DocumentEvent();

  private: DocumentWindow* view() const { return view_.get(); }

  DISALLOW_COPY_AND_ASSIGN(DocumentEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_document_event_h)
