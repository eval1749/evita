// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_document_event_h)
#define INCLUDE_evita_dom_events_document_event_h

#include "base/strings/string16.h"
#include "evita/dom/events/event.h"

namespace dom {

class DocumentEventInit;
class Window;

class DocumentEvent : public v8_glue::Scriptable<DocumentEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(DocumentEvent)

  private: int error_code_;

  public: DocumentEvent(const base::string16& type,
                    const DocumentEventInit& init_dict);
  public: virtual ~DocumentEvent();

  public: int error_code() const { return error_code_; }

  DISALLOW_COPY_AND_ASSIGN(DocumentEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_document_event_h)
