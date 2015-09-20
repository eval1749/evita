// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_DOCUMENT_EVENT_H_
#define EVITA_DOM_EVENTS_DOCUMENT_EVENT_H_

#include "base/strings/string16.h"
#include "evita/dom/events/event.h"
#include "evita/gc/member.h"

namespace dom {

class DocumentEventInit;
class DocumentWindow;

namespace bindings {
class DocumentEventClass;
}

class DocumentEvent final : public v8_glue::Scriptable<DocumentEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(DocumentEvent)

 public:
  // Expose for|DocumentWindow|.
  DocumentEvent(const base::string16& type, const DocumentEventInit& init_dict);
  ~DocumentEvent() final;

 private:
  friend class bindings::DocumentEventClass;

  explicit DocumentEvent(const base::string16& type);

  DocumentWindow* view() const { return view_.get(); }

  gc::Member<DocumentWindow> view_;

  DISALLOW_COPY_AND_ASSIGN(DocumentEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_DOCUMENT_EVENT_H_
