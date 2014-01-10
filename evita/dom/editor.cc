// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor.h"

#include "base/callback.h"
#include "evita/cm_CmdProc.h"
#include "evita/gc/local.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_command.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/script_thread.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/script_callback.h"

namespace dom {

namespace {

const base::char16 kVersion[] = L"5.0";

//////////////////////////////////////////////////////////////////////
//
// EditorClass
//
class EditorClass : public v8_glue::WrapperInfo {
  public: EditorClass() : v8_glue::WrapperInfo("Editor") {
  }
  public: ~EditorClass() = default;

  // TODO(yosi): Until we enable |new EditorWindow()|, we use
  // |editor.createFrame()|.
  private: static v8::Handle<v8::Object> CreateFrame() {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
    ASSERT_DOM_LOCKED();
    auto window = gc::MakeLocal(new EditorWindow());
    return window->GetWrapper(v8::Isolate::GetCurrent());
  }

  private: static void GetFilenameForLoad(Window* window,
                                          const base::string16& dir_path,
                                          v8::Handle<v8::Function> callback) {
    ScriptController::instance()->view_delegate()->GetFilenameForLoad(
        window->widget_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForLoadCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static void GetFilenameForSave(Window* window,
                                          const base::string16& dir_path,
                                          v8::Handle<v8::Function> callback) {
    ScriptController::instance()->view_delegate()->GetFilenameForSave(
        window->widget_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForSaveCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static void MessageBox(Window* window, const base::string16& message,
                                 const base::string16& title, int flags,
                                 v8::Handle<v8::Function> callback) {
    ScriptController::instance()->view_delegate()->MessageBox(
        window->widget_id(), message, title, flags,
        v8_glue::ScriptCallback<ViewDelegate::MessageBoxCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static Editor* NewEditor() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of Editor.");
    return nullptr;
  }

  private: static void SetKeyBinding(int key_code,
                                    v8::Handle<v8::Object> command) {
    ASSERT_DOM_LOCKED();
    if (!Command::g_pGlobalBinds) {
      // During DOM unit testing, |g_GlobalBinds| isn't initialized.
      return;
    }
    Command::g_pGlobalBinds->Bind(key_code, new ScriptCommand(command));
  }

  private: static base::string16 version() {
    return kVersion;
  }

  // WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &EditorClass::NewEditor);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("createFrame", &EditorClass::CreateFrame)
      .SetMethod("getFilenameForLoad_", &EditorClass::GetFilenameForLoad)
      .SetMethod("getFilenameForSave_", &EditorClass::GetFilenameForSave)
      .SetMethod("messageBox_", &EditorClass::MessageBox)
      .SetMethod("setKeyBinding_", &EditorClass::SetKeyBinding)
      .SetProperty("version", &EditorClass::version)
      .Build();
  }
};

}  // namespace

Editor::Editor() {
}

Editor::~Editor() {
}

v8_glue::WrapperInfo* Editor::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EditorClass, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
