// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_controller.h"

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
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

namespace v8Strings {
void Init(v8::Isolate* isolate);
}

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

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

void MessageBoxCallback(int) {
}

bool suppress_message_box;

void MessageBox(const base::string16& message, int flags) {
  DVLOG(0) << message;
  if (suppress_message_box)
    return;
  ScriptController::instance()->view_delegate()->MessageBox(kInvalidWindowId,
    message, L"System Message", flags, base::Bind(MessageBoxCallback));
}

void GcEpilogueCallback(v8::GCType type, v8::GCCallbackFlags flags) {
  auto text = base::StringPrintf(L"GC finished type=%d flags=0x%X",
                                 type, flags);
  MessageBox(text, MB_ICONINFORMATION);
  dom::Lock::instance()->Release(FROM_HERE);
}

void GcPrologueCallback(v8::GCType type, v8::GCCallbackFlags flags) {
  auto text = base::StringPrintf(L"GC Started type=%d flags=%X",
                                 type, flags);
  MessageBox(text, MB_ICONINFORMATION);
  dom::Lock::instance()->Acquire(FROM_HERE);
}

void MessageCallback(v8::Handle<v8::Message> message,
                       v8::Handle<v8::Value> error) {
  auto text = base::StringPrintf(
      L"Exeption: %ls\n"
      L"Source: %ls\n"
      L"Source name: %ls(%d)\n",
      V8ToString(error).c_str(), V8ToString(message->GetSourceLine()).c_str(),
      V8ToString(message->GetScriptResourceName()).c_str(),
      message->GetLineNumber());
  auto const stack_trace = message->GetStackTrace();
  if (!stack_trace.IsEmpty()) {
    text += L"Stack trace:\n";
    auto const length = static_cast<size_t>(stack_trace->GetFrameCount());
    for (auto index = 0u; index < length; ++index) {
      auto const frame = stack_trace->GetFrame(index);
      text += base::StringPrintf(L"  at %ls (%ls(%d))\n",
          V8ToString(frame->GetFunctionName()).c_str(),
          V8ToString(frame->GetScriptName()).c_str(), frame->GetLineNumber());
    }
  }
  MessageBox(text, MB_ICONERROR);
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
      event_handler_(new EventHandler(this)),
      testing_(false),
      view_delegate_(view_delegate) {
  view_delegate_->RegisterViewEventHandler(event_handler_.get());
  v8::V8::InitializeICU();
  auto const isolate = isolate_holder_.isolate();
  isolate->Enter();

  v8::V8::SetCaptureStackTraceForUncaughtExceptions(true);
  v8::V8::AddMessageListener(MessageCallback);
  v8::V8::AddGCPrologueCallback(GcPrologueCallback);
  v8::V8::AddGCEpilogueCallback(GcEpilogueCallback);

  // See v8/src/flag-definitions.h
  // Note: |EnsureV8Initialized()| in "gin/isolate_holder.cc" also sets
  // flags.
  char flags[] =
      "--use_strict"
      " --harmony"
      " --harmony_typeof"
      " --trace_exception";
  v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
}

ScriptController::~ScriptController() {
  isolate_holder_.isolate()->Exit();
  script_controller = nullptr;
}

v8::Handle<v8::Context> ScriptController::context() const {
  if (!testing_context_->IsEmpty())
    return testing_context_.NewLocal(v8::Isolate::GetCurrent());
  return  context_holder_.context();
}

ScriptController* ScriptController::instance() {
  DCHECK(script_controller);
  return script_controller;
}

v8::Isolate* ScriptController::isolate() const {
  return const_cast<ScriptController*>(this)->isolate_holder_.isolate();
}

void ScriptController::set_testing_context(v8::Handle<v8::Context> context) {
  testing_context_.Reset(v8::Isolate::GetCurrent(), context);
}

ViewDelegate* ScriptController::view_delegate() const {
  DCHECK(view_delegate_);
  return view_delegate_;
}

void ScriptController::DidStartHost() {
  // We should prevent UI thread to access DOM.
  DOM_AUTO_LOCK_SCOPE();
  auto const isolate = this->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context());
  LoadJsLibrary();
  if (testing_)
    return;
  auto result = Evaluate(L"editors.start([]);");
  CHECK(result.exception.empty()) << result.exception << std::endl <<
      "line: " << result.line_number;
}

EvaluateResult ScriptController::Evaluate(const base::string16& script_text) {
  auto const isolate = isolate_holder_.isolate();
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::New(
      gin::StringToV8(isolate, script_text)->ToString(),
      gin::StringToV8(isolate, "__eval__")->ToString());
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
  auto const stack_trace = try_catch.StackTrace();
  if (stack_trace.IsEmpty()) {
    DVLOG(0) << "Unhandled exception " << V8ToString(try_catch.Exception());
    return;
  }
  DVLOG(0) << V8ToString(stack_trace);
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
  v8::Handle<v8::Value> js_filename = gin::StringToV8(isolate, filename);
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
  suppress_message_box = true;
  script_controller = new ScriptController(view_delegate);
  auto const isolate = script_controller->isolate();
  v8::HandleScope handle_scope(isolate);
  v8Strings::Init(isolate);
  auto context = v8::Context::New(isolate, nullptr,
      Global::instance()->object_template(isolate));
  script_controller->context_holder_.SetContext(context);
  suppress_message_box = false;
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
  suppress_message_box = true;
  return script_controller;
}

void ScriptController::ThrowError(const std::string& message) {
  auto isolate = isolate_holder_.isolate();
  isolate->ThrowException(v8::Exception::Error(
      gin::StringToV8(isolate, message)));
}

void ScriptController::ThrowException(v8::Handle<v8::Value> exception) {
  auto isolate = isolate_holder_.isolate();
  isolate->ThrowException(exception);
}

void ScriptController::WillDestroyHost() {
  view_delegate_ = nullptr;
}

}  // namespace dom
