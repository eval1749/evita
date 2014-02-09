// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/document_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/document_event_init.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// DocumentEventClass
//
class DocumentEventClass :
    public v8_glue::DerivedWrapperInfo<DocumentEvent, Event> {

  public: explicit DocumentEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~DocumentEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &DocumentEventClass::NewDocumentEvent);
  }

  private: static DocumentEvent* NewDocumentEvent(const base::string16& type,
      v8_glue::Optional<v8::Handle<v8::Object>> opt_dict) {
    DocumentEventInit init_dict;
    if (!init_dict.Init(opt_dict.value))
      return nullptr;
    return new DocumentEvent(type, init_dict);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("errorCode", &DocumentEvent::error_code);
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentEventClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DocumentEvent
//
DEFINE_SCRIPTABLE_OBJECT(DocumentEvent, DocumentEventClass);

DocumentEvent::DocumentEvent(const base::string16& type,
                     const DocumentEventInit& init_dict)
    : ScriptableBase(type, init_dict), error_code_(init_dict.error_code()) {
}

DocumentEvent::~DocumentEvent() {
}

}  // namespace dom
