// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/global.h"

#include "evita/dom/document.h"
#include "evita/dom/document_window.h"
#include "evita/dom/editor.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/file_path.h"
#include "evita/dom/point.h"
#include "evita/dom/range.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/selection.h"
#include "evita/dom/table_selection.h"
#include "evita/dom/table_window.h"
#include "evita/dom/text_selection.h"
#include "evita/dom/text_window.h"
#include "evita/dom/timer.h"
#include "evita/dom/window.h"

namespace dom {

namespace internal {
v8::Handle<v8::Object> GetUnicodeObject(v8::Isolate* isoalte);
}  // namespace internal

Global::Global() {
}

Global::~Global() {
}

v8::Handle<v8::ObjectTemplate> Global::object_template(v8::Isolate* isolate) {
  if (!object_template_->IsEmpty())
    return object_template_.NewLocal(isolate);

  {
    v8::HandleScope handle_scope(isolate);

    auto templ = v8::ObjectTemplate::New(isolate);

    auto context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
  
    // Note: super class must be installed before subclass.
    v8_glue::Installer<Event>::Run(isolate, templ);
      v8_glue::Installer<FormEvent>::Run(isolate, templ);
      v8_glue::Installer<UiEvent>::Run(isolate, templ);
        v8_glue::Installer<FocusEvent>::Run(isolate, templ);
        v8_glue::Installer<MouseEvent>::Run(isolate, templ);
      v8_glue::Installer<WindowEvent>::Run(isolate, templ);

    v8_glue::Installer<Document>::Run(isolate, templ);
    v8_glue::Installer<Editor>::Run(isolate, templ);
    v8_glue::Installer<FilePath>::Run(isolate, templ);
    v8_glue::Installer<Point>::Run(isolate, templ);
    v8_glue::Installer<Range>::Run(isolate, templ);

    v8_glue::Installer<EventTarget>::Run(isolate, templ);
      v8_glue::Installer<Form>::Run(isolate, templ);
      v8_glue::Installer<FormControl>::Run(isolate, templ);
        v8_glue::Installer<TextFieldControl>::Run(isolate, templ);
      v8_glue::Installer<Window>::Run(isolate, templ);
        v8_glue::Installer<DocumentWindow>::Run(isolate, templ);
          v8_glue::Installer<TableWindow>::Run(isolate, templ);
          v8_glue::Installer<TextWindow>::Run(isolate, templ);
        v8_glue::Installer<EditorWindow>::Run(isolate, templ);

    v8_glue::Installer<Selection>::Run(isolate, templ);
        v8_glue::Installer<TableSelection>::Run(isolate, templ);
        v8_glue::Installer<TextSelection>::Run(isolate, templ);

    v8_glue::Installer<Timer>::Run(isolate, templ);
      v8_glue::Installer<OneShotTimer>::Run(isolate, templ);
      v8_glue::Installer<RepeatingTimer>::Run(isolate, templ);

    v8::Handle<v8::Object> js_unicode = v8::Object::New(isolate);
    templ->Set(gin::StringToV8(isolate, "Unicode"), 
               internal::GetUnicodeObject(isolate));
    object_template_.Reset(isolate, templ);
  }

  return object_template_.NewLocal(isolate);
}

}   // namespace dom
