// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/global.h"

#include "evita/dom/document.h"
#include "evita/dom/document_window.h"
#include "evita/dom/editor.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/events/document_event.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/wheel_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/forms/button_control.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/file_path.h"
#include "evita/dom/modes/config_mode.h"
#include "evita/dom/modes/cxx_mode.h"
#include "evita/dom/modes/haskell_mode.h"
#include "evita/dom/modes/java_mode.h"
#include "evita/dom/modes/javascript_mode.h"
#include "evita/dom/modes/lisp_mode.h"
#include "evita/dom/modes/mason_mode.h"
#include "evita/dom/modes/mode.h"
#include "evita/dom/modes/perl_mode.h"
#include "evita/dom/modes/plain_text_mode.h"
#include "evita/dom/modes/python_mode.h"
#include "evita/dom/modes/xml_mode.h"
#include "evita/dom/os/file.h"
#include "evita/dom/point.h"
#include "evita/dom/range.h"
#include "evita/dom/regexp.h"
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

    #define INSTALL(name) v8_glue::Installer<name>::Run(isolate, templ);
  
    // Note: super class must be installed before subclass.
    INSTALL(Event);
      INSTALL(DocumentEvent);
      INSTALL(FormEvent);
      INSTALL(UiEvent);
        INSTALL(FocusEvent);
        INSTALL(KeyboardEvent);
        INSTALL(MouseEvent);
          INSTALL(WheelEvent);
      INSTALL(WindowEvent);

    INSTALL(Document);
    auto const editor_templ = INSTALL(Editor);
    INSTALL(FilePath);
    INSTALL(Point);
    INSTALL(Range);

    INSTALL(EventTarget);
      INSTALL(Form);
      INSTALL(FormControl);
        INSTALL(ButtonControl);
        INSTALL(CheckboxControl);
        INSTALL(RadioButtonControl);
        INSTALL(TextFieldControl);
      INSTALL(Window);
        INSTALL(DocumentWindow);
          INSTALL(TableWindow);
          INSTALL(TextWindow);
        INSTALL(EditorWindow);

    INSTALL(Mode);
      INSTALL(ConfigMode);
      INSTALL(CxxMode);
      INSTALL(HaskellMode);
      INSTALL(JavaMode);
      INSTALL(JavaScriptMode);
      INSTALL(LispMode);
      INSTALL(MasonMode);
      INSTALL(PerlMode);
      INSTALL(PlainTextMode);
      INSTALL(PythonMode);
      INSTALL(XmlMode);

    INSTALL(Selection);
        INSTALL(TableSelection);
        INSTALL(TextSelection);

    INSTALL(Timer);
      INSTALL(OneShotTimer);
      INSTALL(RepeatingTimer);

    // Editor
    editor_templ->Set(gin::StringToV8(isolate, "RegExp"),
                      RegExp::static_wrapper_info()->
                          GetOrCreateConstructorTemplate(isolate));

    // Os
    auto const os_templ = v8::ObjectTemplate::New(isolate);
    templ->Set(gin::StringToV8(isolate, "Os"), os_templ);
    os_templ->Set(gin::StringToV8(isolate, "File"),
                  os::File::static_wrapper_info()->
                      GetOrCreateConstructorTemplate(isolate));

    // Unicode
    v8::Handle<v8::Object> js_unicode = v8::Object::New(isolate);
    templ->Set(gin::StringToV8(isolate, "Unicode"), 
               internal::GetUnicodeObject(isolate));
    object_template_.Reset(isolate, templ);
  }

  return object_template_.NewLocal(isolate);
}

}   // namespace dom
