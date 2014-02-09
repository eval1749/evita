// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_document_event_init_h)
#define INCLUDE_evita_dom_events_document_event_init_h

#include "evita/dom/events/event_init.h"

namespace dom {

class DocumentEventInit : public EventInit {
  private: int error_code_;

  public: DocumentEventInit();
  public: virtual ~DocumentEventInit();

  public: int error_code() const { return error_code_; }
  public: void set_error_code(int error_code) { error_code_ = error_code; }

  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(DocumentEventInit);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_document_event_init_h)
