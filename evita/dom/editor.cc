// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/editor.h"

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace gin {
bool Converter<domapi::SwitchValue>::FromV8(v8::Isolate* isolate,
                                            v8::Handle<v8::Value> val,
                                            domapi::SwitchValue* out) {
  if (val.IsEmpty())
    return false;
  bool bool_value;
  if (ConvertFromV8(isolate, val, &bool_value)) {
    *out = domapi::SwitchValue(bool_value);
    return true;
  }
  int int_value;
  if (ConvertFromV8(isolate, val, &int_value)) {
    *out = domapi::SwitchValue(int_value);
    return true;
  }
  base::string16 str_value;
  if (ConvertFromV8(isolate, val, &str_value)) {
    *out = domapi::SwitchValue(str_value);
    return true;
  }
  return false;
}

v8::Handle<v8::Value> Converter<domapi::SwitchValue>::ToV8(
    v8::Isolate* isolate,
    const domapi::SwitchValue& val) {
  switch (val.type()) {
    case domapi::SwitchValue::Type::Bool:
      return ConvertToV8(isolate, val.bool_value());
    case domapi::SwitchValue::Type::Int:
      return ConvertToV8(isolate, val.int_value());
    case domapi::SwitchValue::Type::String:
      return ConvertToV8(isolate, val.string_value());
    case domapi::SwitchValue::Type::Void:
      break;
  }
  return v8::Undefined(isolate);
}

bool Converter<domapi::TabData>::FromV8(v8::Isolate* isolate,
                                        v8::Handle<v8::Value> val,
                                        domapi::TabData* out) {
  if (val.IsEmpty() || !val->IsObject())
    return false;
  auto const obj = val->ToObject();
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::icon.Get(isolate)),
                     &out->icon)) {
    return false;
  }
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::state.Get(isolate)),
                     &out->state)) {
    return false;
  }
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::title.Get(isolate)),
                     &out->title)) {
    return false;
  }
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::tooltip.Get(isolate)),
                     &out->tooltip)) {
    return false;
  }
  return true;
}
}  // namespace gin

namespace dom {

namespace {

const base::char16 kVersion[] = L"5.0";

v8::Local<v8::Object> NewRunScriptResult(v8::Isolate* isolate,
                                         v8::Handle<v8::Value> run_value,
                                         const v8::TryCatch& try_catch) {
  auto const result = v8::Object::New(isolate);
  if (try_catch.HasCaught()) {
    result->Set(gin::StringToV8(isolate, "exception"), try_catch.Exception());
    auto const message = try_catch.Message();
    if (!message.IsEmpty()) {
      result->Set(gin::StringToV8(isolate, "stackTrace"),
                  message->GetStackTrace().IsEmpty()
                      ? v8::Array::New(isolate, 0)
                      : message->GetStackTrace()->AsArray());
      result->Set(gin::StringToV8(isolate, "stackTraceString"),
                  try_catch.StackTrace().IsEmpty()
                      ? gin::ConvertToV8(isolate, base::string16())
                      : try_catch.StackTrace());
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

v8::Handle<v8::Object> RunScriptInternal(const base::string16& script_text,
                                         const base::string16& file_name) {
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  v8::TryCatch try_catch;
  v8::ScriptOrigin script_origin(
      gin::StringToV8(isolate, file_name)->ToString());
  auto const script = v8::Script::Compile(
      gin::StringToV8(isolate, script_text)->ToString(), &script_origin);
  if (script.IsEmpty()) {
    return runner_scope.Escape(
        NewRunScriptResult(isolate, v8::Handle<v8::Value>(), try_catch));
  }
  auto const run_value = script->Run();
  if (run_value.IsEmpty()) {
    return runner_scope.Escape(
        NewRunScriptResult(isolate, v8::Handle<v8::Value>(), try_catch));
  }
  return runner_scope.Escape(NewRunScriptResult(isolate, run_value, try_catch));
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor() {}

Editor::~Editor() {}

base::string16 Editor::v8_version() {
  return base::ASCIIToUTF16(v8::V8::GetVersion());
}

base::string16 Editor::version() {
  return kVersion;
}

v8::Handle<v8::Promise> Editor::CheckSpelling(
    const base::string16& word_to_check) {
  return PromiseResolver::SlowCall(base::Bind(
      &domapi::IoDelegate::CheckSpelling,
      base::Unretained(ScriptHost::instance()->io_delegate()), word_to_check));
}

bool Editor::CollectGarbage(int hint) {
  if (hint >= 1 && hint <= 1000)
    return ScriptHost::instance()->isolate()->IdleNotification(hint);
  ScriptHost::instance()->isolate()->LowMemoryNotification();
  return false;
}

bool Editor::CollectGarbage() {
  ScriptHost::instance()->isolate()->LowMemoryNotification();
  return false;
}

v8::Handle<v8::Promise> Editor::GetFileNameForLoad(
    Window* window,
    const base::string16& dir_path) {
  return PromiseResolver::SlowCall(base::Bind(
      &ViewDelegate::GetFileNameForLoad,
      base::Unretained(ScriptHost::instance()->view_delegate()),
      window ? window->window_id() : dom::kInvalidWindowId, dir_path));
}

v8::Handle<v8::Promise> Editor::GetFileNameForSave(
    Window* window,
    const base::string16& dir_path) {
  return PromiseResolver::SlowCall(base::Bind(
      &ViewDelegate::GetFileNameForSave,
      base::Unretained(ScriptHost::instance()->view_delegate()),
      window ? window->window_id() : dom::kInvalidWindowId, dir_path));
}

base::string16 Editor::GetMetrics(const base::string16& name) {
  return ScriptHost::instance()->view_delegate()->GetMetrics(name);
}

v8::Handle<v8::Promise> Editor::GetSpellingSuggestions(
    const base::string16& wrong_word) {
  return PromiseResolver::SlowCall(base::Bind(
      &domapi::IoDelegate::GetSpellingSuggestions,
      base::Unretained(ScriptHost::instance()->io_delegate()), wrong_word));
}

domapi::SwitchValue Editor::GetSwitch(const base::string16& name) {
  return ScriptHost::instance()->view_delegate()->GetSwitch(name);
}

std::vector<base::string16> Editor::GetSwitchNames() {
  return ScriptHost::instance()->view_delegate()->GetSwitchNames();
}

v8::Handle<v8::Promise> Editor::MessageBox(Window* maybe_window,
                                           const base::string16& message,
                                           int flags,
                                           const base::string16& title) {
  return PromiseResolver::SlowCall(
      base::Bind(&ViewDelegate::MessageBox,
                 base::Unretained(ScriptHost::instance()->view_delegate()),
                 maybe_window ? maybe_window->window_id() : kInvalidWindowId,
                 message, title, flags));
}

v8::Handle<v8::Promise> Editor::MessageBox(Window* maybe_window,
                                           const base::string16& message,
                                           int flags) {
  return MessageBox(maybe_window, message, flags, base::string16());
}

v8::Handle<v8::Object> Editor::RunScript(const base::string16& script_text,
                                         const base::string16& file_name) {
  if (file_name == L"*javascript*") {
    SuppressMessageBoxScope suppress_messagebox_scope;
    return RunScriptInternal(script_text, file_name);
  }
  return RunScriptInternal(script_text, file_name);
}

v8::Handle<v8::Object> Editor::RunScript(const base::string16& script_text) {
  return RunScript(script_text, L"__runscript__");
}

void Editor::SetSwitch(const base::string16& name,
                       const domapi::SwitchValue& new_value) {
  return ScriptHost::instance()->view_delegate()->SetSwitch(name, new_value);
}

void Editor::SetTabData(Window* window, const domapi::TabData& tab_data) {
  ScriptHost::instance()->view_delegate()->SetTabData(window->window_id(),
                                                      tab_data);
}

}  // namespace dom
