// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor.h"

#include "base/bind.h"
#include "base/callback.h"
#include "evita/gc/local.h"
#include "evita/dom/promise_deferred.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/promise_callback.h"
#include "evita/v8_glue/runner.h"
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

  private: static v8::Handle<v8::Value> CheckSpelling(
      const base::string16& word_to_check);

  private: static void GetFilenameForLoad(Window* window,
                                          const base::string16& dir_path,
                                          v8::Handle<v8::Function> callback) {
    auto const runner = ScriptController::instance()->runner();
    ScriptController::instance()->view_delegate()->GetFilenameForLoad(
        window->window_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForLoadCallback>::New(
            runner->GetWeakPtr(), callback));
  }

  private: static void GetFilenameForSave(Window* window,
                                          const base::string16& dir_path,
                                          v8::Handle<v8::Function> callback) {
    auto const runner = ScriptController::instance()->runner();
    ScriptController::instance()->view_delegate()->GetFilenameForSave(
        window->window_id(), dir_path,
        v8_glue::ScriptCallback<ViewDelegate::GetFilenameForSaveCallback>::New(
            runner->GetWeakPtr(), callback));
  }

  private: static base::string16 GetMetrics(const base::string16& name) {
    return ScriptController::instance()->view_delegate()->GetMetrics(name);
  }

  private: static  v8::Handle<v8::Value> GetSpellingSuggestions(
      const base::string16& wrong_word);

  private: static void MessageBox(v8_glue::Nullable<Window> maybe_window,
                                 const base::string16& message, int flags,
                                 const base::string16& title,
                                 v8::Handle<v8::Function> callback) {
    auto const runner = ScriptController::instance()->runner();
    ScriptController::instance()->view_delegate()->MessageBox(
        maybe_window ? maybe_window->window_id() : kInvalidWindowId,
        message, title, flags,
        v8_glue::ScriptCallback<ViewDelegate::MessageBoxCallback>::New(
            runner->GetWeakPtr(), callback));
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
      auto const message = try_catch.Message();
      if (!message.IsEmpty()) {
        result->Set(gin::StringToV8(isolate, "stackTrace"),
            message->GetStackTrace().IsEmpty() ? v8::Array::New(isolate, 0) :
              message->GetStackTrace()->AsArray());
        result->Set(gin::StringToV8(isolate, "stackTraceString"),
            try_catch.StackTrace().IsEmpty() ?
                gin::ConvertToV8(isolate, base::string16()) :
                try_catch.StackTrace());
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
      }
    } else {
      result->Set(gin::StringToV8(isolate, "value"), run_value);
    }
    return result;
  }

  private: static v8::Handle<v8::Object> RunScript(
      const base::string16& script_text,
      v8_glue::Optional<base::string16> opt_file_name) {
    auto const file_name = opt_file_name.get(L"__runscript__");
    if (file_name == L"*javascript*") {
      SuppressMessageBoxScope suppress_messagebox_scope;
      return RunScriptInternal(script_text, file_name);
    }
    return RunScriptInternal(script_text, file_name);
  }

  private: static v8::Handle<v8::Object> RunScriptInternal(
      const base::string16& script_text,
      const base::string16& file_name) {
    auto const runner = ScriptController::instance()->runner();
    auto const isolate = runner->isolate();
    v8_glue::Runner::EscapableHandleScope runner_scope(runner);
    v8::TryCatch try_catch;
    try_catch.SetCaptureMessage(true);
    try_catch.SetVerbose(true);
    auto const script = v8::Script::New(
        gin::StringToV8(isolate, script_text)->ToString(),
        gin::StringToV8(isolate, file_name)
            ->ToString());
    if (script.IsEmpty()) {
      return runner_scope.Escape(NewRunScriptResult(isolate,
          v8::Handle<v8::Value>(), try_catch));
    }
    auto const run_value = script->Run();
    if (run_value.IsEmpty()) {
      return runner_scope.Escape(NewRunScriptResult(isolate,
          v8::Handle<v8::Value>(), try_catch));
    }
    return runner_scope.Escape(NewRunScriptResult(isolate, run_value,
        try_catch));
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
      .SetMethod("checkSpelling", &EditorClass::CheckSpelling)
      .SetMethod("getFilenameForLoad_", &EditorClass::GetFilenameForLoad)
      .SetMethod("getFilenameForSave_", &EditorClass::GetFilenameForSave)
      .SetMethod("getSpellingSuggestions",
          &EditorClass::GetSpellingSuggestions)
      .SetMethod("messageBox_", &EditorClass::MessageBox)
      .SetMethod("metrics", &EditorClass::GetMetrics)
      .SetMethod("runScript", &EditorClass::RunScript)
      .SetProperty("version", &EditorClass::version)
      .Build();
  }

  DISALLOW_COPY_AND_ASSIGN(EditorClass);
};

v8::Handle<v8::Value> EditorClass::CheckSpelling(
    const base::string16& word_to_check) {
  auto const promise_deferred = PromiseDeferred::Call(base::Bind(
      &ViewDelegate::CheckSpelling,
      base::Unretained(ScriptController::instance()->view_delegate()),
      word_to_check));
  auto const isolate = ScriptController::instance()->isolate();
  return gin::ConvertToV8(isolate, promise_deferred.get());
}

v8::Handle<v8::Value> EditorClass::GetSpellingSuggestions(
    const base::string16& wrong_word) {
  auto const promise_deferred = PromiseDeferred::Call(base::Bind(
      &ViewDelegate::GetSpellingSuggestions,
      base::Unretained(ScriptController::instance()->view_delegate()),
      wrong_word));
  auto const isolate = ScriptController::instance()->isolate();
  return gin::ConvertToV8(isolate, promise_deferred.get());
}

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
