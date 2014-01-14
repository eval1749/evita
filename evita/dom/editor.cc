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
  public: EditorClass(const char* name)
      : v8_glue::WrapperInfo(name) {
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
        window->window_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForLoadCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static void GetFilenameForSave(Window* window,
                                          const base::string16& dir_path,
                                          v8::Handle<v8::Function> callback) {
    ScriptController::instance()->view_delegate()->GetFilenameForSave(
        window->window_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForSaveCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static void MessageBox(Window* window, const base::string16& message,
                                 const base::string16& title, int flags,
                                 v8::Handle<v8::Function> callback) {
    ScriptController::instance()->view_delegate()->MessageBox(
        window->window_id(), message, title, flags,
        v8_glue::ScriptCallback<ViewDelegate::MessageBoxCallback>::New(
            v8::Isolate::GetCurrent(), callback));
  }

  private: static Editor* NewEditor() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of Editor.");
    return nullptr;
  }

  private: static v8::Local<v8::Object> NewRunScriptResult(
      v8::Isolate* isolate, v8::Handle<v8::Value> run_value,
      const v8::TryCatch& try_catch) {
    auto const result = v8::Object::New(isolate);
    if (try_catch.HasCaught()) {
      result->Set(gin::StringToV8(isolate, "exception"),
          try_catch.Exception());
      result->Set(gin::StringToV8(isolate, "stackTrace"),
          try_catch.StackTrace());
      auto const message = try_catch.Message();
      result->Set(gin::StringToV8(isolate, "lineNumber"),
          gin::ConvertToV8(isolate, message->GetLineNumber()));
      result->Set(gin::StringToV8(isolate, "start"),
          gin::ConvertToV8(isolate, message->GetStartPosition()));
      result->Set(gin::StringToV8(isolate, "end"),
          gin::ConvertToV8(isolate, message->GetEndPosition()));
      result->Set(gin::StringToV8(isolate, "startColumn"),
          gin::ConvertToV8(isolate, message->GetStartColumn()));
      result->Set(gin::StringToV8(isolate, "endColumn"),
          gin::ConvertToV8(isolate, message->GetEndColumn()));
    } else {
      result->Set(gin::StringToV8(isolate, "value"), run_value);
    }
    return result;
  }

  private: static v8::Handle<v8::Object> RunScript(
      const base::string16& script_text) {
    auto const isolate = ScriptController::instance()->isolate();
    v8::EscapableHandleScope handle_scope(isolate);
    auto const context = ScriptController::instance()->context();
    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch;
    auto const script = v8::Script::New(
        gin::StringToV8(isolate, script_text)->ToString(),
        gin::StringToV8(isolate, "(runScript)")->ToString());
    if (script.IsEmpty()) {
      return handle_scope.Escape(NewRunScriptResult(isolate,
          v8::Handle<v8::Value>(), try_catch));
    }
    auto const run_value = script->Run();
    if (run_value.IsEmpty()) {
      return handle_scope.Escape(NewRunScriptResult(isolate,
          v8::Handle<v8::Value>(), try_catch));
    }
    return handle_scope.Escape(NewRunScriptResult(isolate, run_value,
        try_catch));
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
      .SetMethod("runScript", &EditorClass::RunScript)
      .SetMethod("setKeyBinding_", &EditorClass::SetKeyBinding)
      .SetProperty("version", &EditorClass::version)
      .Build();
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//
DEFINE_SCRIPTABLE_OBJECT(Editor, EditorClass);

Editor::Editor() {
}

Editor::~Editor() {
}

}  // namespace dom
