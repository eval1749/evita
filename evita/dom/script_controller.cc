// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_controller.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "evita/dom/document.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/events/event_handler.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/per_isolate_data.h"

namespace dom {

namespace internal {
const base::string16& GetJsLibSource();
}  // namespace internal

namespace {

v8::Handle<v8::Value> GetOpenFileHandler(
    v8::Handle<v8::Context> context, WindowId window_id) {
  auto const isolate = context->GetIsolate();
  if (window_id == kInvalidWindowId)
    return context->Global()->Get(gin::StringToV8(isolate, "editor"));

  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << "OpenFile: No suche window " << window_id;
    return v8::Handle<v8::Value>();
  }
  return window->GetWrapper(isolate);
}

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        string_value.length());
}

// TODO(yosi) We will remove EvaluateResult once V8Console in JS.
EvaluateResult ReportException(const v8::TryCatch& try_catch) {
  EvaluateResult eval_result;
  auto message = try_catch.Message();
  eval_result.exception = V8ToString(try_catch.Exception());
  if (!message.IsEmpty()) {
    eval_result.script_resource_name = V8ToString(
        message->GetScriptResourceName());
    eval_result.line_number = message->GetLineNumber();
    eval_result.source_line = V8ToString(message->GetSourceLine());
    eval_result.start_column = message->GetStartColumn();
    eval_result.end_column = message->GetEndColumn();
  }

  auto stack_trace = try_catch.StackTrace();
  if (!stack_trace.IsEmpty() && stack_trace->IsArray()) {
    auto array = stack_trace.As<v8::Array>();
    eval_result.stack_trace.resize(array->Length());
    auto index = 0u;
    for (auto dst: eval_result.stack_trace) {
      auto entry = array->CloneElementAt(index);
      dst = V8ToString(entry);
      ++index;
    }
  }
  return eval_result;
}

ScriptController* script_controller;

}   // namespace

//////////////////////////////////////////////////////////////////////
//
// EvaluateResult
//
EvaluateResult::EvaluateResult(const base::string16& value)
    : value(value), line_number(0), start_column(0), end_column(0) {
}

EvaluateResult::EvaluateResult()
    : EvaluateResult(base::string16()) {
}

//////////////////////////////////////////////////////////////////////
//
// ScriptController
//
ScriptController::ScriptController(ViewDelegate* view_delegate)
    : context_holder_(isolate_holder_.isolate()),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          event_handler_(new EventHandler(this))),
      testing_(false),
      view_delegate_(view_delegate) {
  view_delegate_->RegisterViewEventHandler(event_handler_.get());
  auto const isolate = isolate_holder_.isolate();
  isolate->Enter();
  v8::HandleScope handle_scope(isolate);
  auto context = v8::Context::New(isolate, nullptr,
      Global::instance()->object_template(isolate));
  context_holder_.SetContext(context);
  context->Enter();
}

ScriptController::~ScriptController() {
  {
    v8::HandleScope handle_scope(isolate_holder_.isolate());
    context_holder_.context()->Exit();
  }
  isolate_holder_.isolate()->Exit();
  script_controller = nullptr;
}

ScriptController* ScriptController::instance() {
  DCHECK(script_controller);
  return script_controller;
}

v8::Isolate* ScriptController::isolate() const {
  return const_cast<ScriptController*>(this)->isolate_holder_.isolate();
}

ViewDelegate* ScriptController::view_delegate() const {
  DCHECK(view_delegate_);
  return view_delegate_;
}

void ScriptController::DidStartHost() {
  // We should prevent UI thread to access DOM.
  DOM_AUTO_LOCK_SCOPE();
  LoadJsLibrary();
  if (testing_)
    return;
  auto result = Evaluate(
    L"(function() {\n"
    L"var doc = new Document('*scratch*');\n"
    L"var range = new Range(doc);\n"
    L"var editor_window = new EditorWindow();\n"
    L"var text_window = new TextWindow(range);\n"
    L"editor_window.add(text_window);\n"
    L"editor_window.realize();\n"
    L"})();");
  CHECK(result.exception.empty()) << result.exception << std::endl <<
      "line: " << result.line_number;
}

EvaluateResult ScriptController::Evaluate(const base::string16& script_text) {
  auto const isolate = isolate_holder_.isolate();
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(
      gin::StringToV8(isolate, script_text)->ToString(),
      gin::StringToV8(isolate, "(eval)")->ToString());
  if (script.IsEmpty()) {
    return ReportException(try_catch);
  }
  v8::Handle<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    DCHECK(try_catch.HasCaught());
    return ReportException(try_catch);
  }

  return EvaluateResult(V8ToString(result->ToString()));
}

void ScriptController::LoadJsLibrary() {
  ASSERT_DOM_LOCKED();
  auto result = Evaluate(internal::GetJsLibSource());
  if (result.exception.empty())
    return;

  DVLOG(0) << "LoadJsLibrary: " << result.exception << " at" <<
    " line:" << result.line_number <<
    " column:" << result.start_column <<
    " source:" << result.source_line;
  NOTREACHED();
}

void ScriptController::LogException(const v8::TryCatch& try_catch) {
  DVLOG(0) << "Unhandled exception " << V8ToString(try_catch.Exception());
}

void ScriptController::OpenFile(WindowId window_id,
                                const base::string16& filename){
  auto const isolate = isolate_holder_.isolate();
  v8::HandleScope handle_scope(isolate);
  auto js_handler = GetOpenFileHandler(context_holder_.context(), window_id);
  if (js_handler.IsEmpty())
    return;
  auto open_file = js_handler->ToObject()->Get(
      gin::StringToV8(isolate, "open"));
  if (!open_file->IsFunction()) {
    DVLOG(0) << "OpenFile: window doesn't have callable openFile property.";
    return;
  }
  auto js_filename = gin::StringToV8(isolate, filename);
  DOM_AUTO_LOCK_SCOPE();
  open_file->ToObject()->CallAsFunction(js_handler, 1, &js_filename);
}

void ScriptController::ResetForTesting() {
  Document::ResetForTesting();
  EditorWindow::ResetForTesting();
  Window::ResetForTesting();
}

ScriptController* ScriptController::Start(ViewDelegate* view_delegate) {
  // Node: Useing Application::instance() starts thread. So, we don't
  // start |ScriptController| in testing. Although, we should remove
  // all |Application::instance()| in DOM world.
  if (script_controller && script_controller->testing_)
    return script_controller;
  DCHECK(!script_controller);
  script_controller = new ScriptController(view_delegate);
  return script_controller;
}

ScriptController* ScriptController::StartForTesting(
    ViewDelegate* view_delegate) {
  if (!script_controller) {
    Start(view_delegate)->testing_ = true;
  } else {
    // In testing, view_delegate is gmock'ed object. Each test case passes
    // newly constructed one.
    script_controller->view_delegate_ = view_delegate;
    script_controller->ResetForTesting();
  }
  return script_controller;
}

void ScriptController::ThrowError(const std::string& message) {
  auto isolate = isolate_holder_.isolate();
  isolate->ThrowException(v8::Exception::Error(
      gin::StringToV8(isolate, message)));
}

void ScriptController::WillDestroyHost() {
  view_delegate_ = nullptr;
}

}  // namespace dom
