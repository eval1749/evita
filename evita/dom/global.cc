// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/global.h"

#include "evita/dom/text/document.h"
#include "evita/dom/windows/document_window.h"
#include "evita/dom/editor.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/encoding/text_decoder.h"
#include "evita/dom/encoding/text_encoder.h"
#include "evita/dom/events/document_event.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/events/wheel_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/forms/button_control.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_window.h"
#include "evita/dom/forms/label_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/file_path.h"
#include "evita/dom/text/modes/config_mode.h"
#include "evita/dom/text/modes/cxx_mode.h"
#include "evita/dom/text/modes/haskell_mode.h"
#include "evita/dom/text/modes/java_mode.h"
#include "evita/dom/text/modes/javascript_mode.h"
#include "evita/dom/text/modes/lisp_mode.h"
#include "evita/dom/text/modes/mason_mode.h"
#include "evita/dom/text/modes/mode.h"
#include "evita/dom/text/modes/perl_mode.h"
#include "evita/dom/text/modes/plain_text_mode.h"
#include "evita/dom/text/modes/python_mode.h"
#include "evita/dom/text/modes/xml_mode.h"
#include "evita/dom/text/mutation_observer.h"
#include "evita/dom/text/mutation_record.h"
#include "evita/dom/os/file.h"
#include "evita/dom/os/process.h"
#include "evita/dom/windows/point.h"
#include "evita/dom/text/range.h"
#include "evita/dom/text/regexp.h"
#include "evita/dom/script_host.h"
#include "evita/dom/windows/selection.h"
#include "evita/dom/windows/table_selection.h"
#include "evita/dom/windows/table_window.h"
#include "evita/dom/windows/text_selection.h"
#include "evita/dom/windows/text_window.h"
#include "evita/dom/timer.h"
#include "evita/dom/windows/window.h"

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

    auto global_templ = v8::ObjectTemplate::New(isolate);

    auto context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

    #define INSTALL(name) v8_glue::Installer<name>::Run(isolate, global_templ)
  
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

    auto const editor_templ = INSTALL(Editor);
    INSTALL(FilePath);
    INSTALL(Point);
    INSTALL(Range);

    INSTALL(EventTarget);
      INSTALL(Document);
      INSTALL(ViewEventTarget);
        INSTALL(Form);
        INSTALL(FormControl);
          INSTALL(ButtonControl);
          INSTALL(CheckboxControl);
          INSTALL(LabelControl);
          INSTALL(RadioButtonControl);
          INSTALL(TextFieldControl);
        INSTALL(Window);
          INSTALL(DocumentWindow);
            INSTALL(TableWindow);
            INSTALL(TextWindow);
          INSTALL(EditorWindow);
          INSTALL(FormWindow);
    INSTALL(TextFieldSelection);

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

    INSTALL(MutationObserver);
    INSTALL(MutationRecord);

    INSTALL(Selection);
        INSTALL(TableSelection);
        INSTALL(TextSelection);

    INSTALL(TextDecoder);
    INSTALL(TextEncoder);

    INSTALL(Timer);
      INSTALL(OneShotTimer);
      INSTALL(RepeatingTimer);

    #define INSTALL_IN(templ, name) \
        templ->Set(gin::StringToV8(isolate, #name), \
                   name::static_wrapper_info()-> \
                      GetOrCreateConstructorTemplate(isolate))

    // Editor
    INSTALL_IN(editor_templ, RegExp);

    // Os
    auto const os_templ = v8::ObjectTemplate::New(isolate);
    global_templ->Set(gin::StringToV8(isolate, "Os"), os_templ);
    typedef os::AbstractFile AbstractFile;
    typedef os::File File;
    typedef os::Process Process;
    INSTALL_IN(os_templ, AbstractFile);
    INSTALL_IN(os_templ, File);
    INSTALL_IN(os_templ, Process);

    // Unicode
    auto const js_unicode = v8::Object::New(isolate);
    global_templ->Set(gin::StringToV8(isolate, "Unicode"),
                      internal::GetUnicodeObject(isolate));

    // Global template is ready now.
    object_template_.Reset(isolate, global_templ);
  }

  return object_template_.NewLocal(isolate);
}

}   // namespace dom
