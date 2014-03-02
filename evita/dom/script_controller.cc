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
#include "evita/v8_glue/runner.h"

namespace dom {

namespace internal {
bool GetJsLibFiles(v8_glue::Runner* runner);
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

v8::Handle<v8::Value> GetOpenFileHandler(v8_glue::Runner* runner,
                                         WindowId window_id) {
  auto const isolate = runner->isolate();
  if (window_id == kInvalidWindowId)
    return runner->global()->Get(gin::StringToV8(isolate, "editor"));

  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << "OpenFile: No suche window " << window_id;
    return v8::Handle<v8::Value>();
  }
  return window->GetWrapper(isolate);
}

void MessageBoxCallback(int) {
}

bool need_unlock_after_gc;
int suppress_message_box;

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
  if (need_unlock_after_gc)
    dom::Lock::instance()->Release(FROM_HERE);
}

void GcPrologueCallback(v8::GCType type, v8::GCCallbackFlags flags) {
  auto text = base::StringPrintf(L"GC Started type=%d flags=%X",
                                 type, flags);
  MessageBox(text, MB_ICONINFORMATION);
  need_unlock_after_gc = !dom::Lock::instance()->locked_by_dom();
  if (need_unlock_after_gc)
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
// SuppressMessageBoxScope
SuppressMessageBoxScope::SuppressMessageBoxScope() {
  ++suppress_message_box;
}

SuppressMessageBoxScope::~SuppressMessageBoxScope() {
  DCHECK_GE(suppress_message_box, 1);
  --suppress_message_box;
}

//////////////////////////////////////////////////////////////////////
//
// ScriptController
//
ScriptController::ScriptController(ViewDelegate* view_delegate,
                                   domapi::IoDelegate* io_delegate)
    : event_handler_(new EventHandler(this)),
      io_delegate_(io_delegate),
      started_(false),
      testing_(false),
      testing_runner_(nullptr),
      view_delegate_(view_delegate) {
  view_delegate_->RegisterViewEventHandler(event_handler_.get());
  v8::V8::InitializeICU();

  // When call TerminateExecution(), assertion failure is occured at
  // |OptimizedFrame::Summarize| in "v8/src/frame.cc" with opcode == REGISTER.
  // v8::V8::SetCaptureStackTraceForUncaughtExceptions(true);
  v8::V8::AddMessageListener(MessageCallback);
  v8::V8::AddGCPrologueCallback(GcPrologueCallback);
  v8::V8::AddGCEpilogueCallback(GcEpilogueCallback);

  // See v8/src/flag-definitions.h
  // Note: |EnsureV8Initialized()| in "gin/isolate_holder.cc" also sets
  // flags.
  char flags[] =
      "--use_strict"
      " --harmony"
      " --harmony_typeof";
  v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
}

ScriptController::~ScriptController() {
  script_controller = nullptr;
}

ScriptController* ScriptController::instance() {
  DCHECK(script_controller);
  return script_controller;
}

v8::Isolate* ScriptController::isolate() const {
  return const_cast<ScriptController*>(this)->isolate_holder_.isolate();
}

v8_glue::Runner* ScriptController::runner() const {
  return testing_runner_ ? testing_runner_ : runner_.get();
}

void ScriptController::set_testing_runner(v8_glue::Runner* runner) {
  testing_runner_ = runner;
}

ViewDelegate* ScriptController::view_delegate() const {
  DCHECK(view_delegate_);
  return view_delegate_;
}

void ScriptController::DidStartHost() {
  // We should prevent UI thread to access DOM.
  DOM_AUTO_LOCK_SCOPE();
  v8_glue::Runner::Scope runner_scope(runner());
  if (!internal::GetJsLibFiles(runner()))
    return;
  if (testing_)
    return;
  runner()->Run(L"editors.start([]);", L"__start__");
  started_ = true;
}

void ScriptController::OpenFile(WindowId window_id,
                                const base::string16& filename){
  v8_glue::Runner::Scope runner_scope(runner());
  auto const isolate = runner()->isolate();
  auto const js_handler = GetOpenFileHandler(runner(), window_id);
  if (js_handler.IsEmpty())
    return;
  auto const open_file = js_handler->ToObject()->Get(
      gin::StringToV8(isolate, "open"));
  if (!open_file->IsFunction()) {
    DVLOG(0) << "OpenFile: window doesn't have callable open property.";
    return;
  }
  v8::Handle<v8::Value> js_filename = gin::StringToV8(isolate, filename);
  DOM_AUTO_LOCK_SCOPE();
  runner()->Call(open_file, js_handler, js_filename);
}

void ScriptController::ResetForTesting() {
  Document::ResetForTesting();
  EditorWindow::ResetForTesting();
  Window::ResetForTesting();
}

ScriptController* ScriptController::Start(ViewDelegate* view_delegate,
                                          domapi::IoDelegate* io_deleage) {
  // Node: Useing Application::instance() starts thread. So, we don't
  // start |ScriptController| in testing. Although, we should remove
  // all |Application::instance()| in DOM world.
  if (script_controller && script_controller->testing_)
    return script_controller;
  DCHECK(!script_controller);
  SuppressMessageBoxScope suppress_messagebox_scope;
  script_controller = new ScriptController(view_delegate, io_deleage);
  auto const isolate = script_controller->isolate();
  auto const runner = new v8_glue::Runner(isolate, script_controller);
  script_controller->runner_.reset(runner);
  v8_glue::Runner::Scope runner_scope(runner);
  v8Strings::Init(isolate);
  return script_controller;
}

ScriptController* ScriptController::StartForTesting(
    ViewDelegate* view_delegate, domapi::IoDelegate* io_delegate) {
  if (!script_controller) {
    Start(view_delegate, io_delegate)->testing_ = true;
  } else {
    // In testing, view_delegate is gmock'ed object. Each test case passes
    // newly constructed one.
    script_controller->io_delegate_ = io_delegate;
    script_controller->view_delegate_ = view_delegate;
    script_controller->ResetForTesting();
  }
  suppress_message_box = 1;
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

v8::Handle<v8::ObjectTemplate> ScriptController::GetGlobalTemplate(
    v8_glue::Runner* runner) {
  return Global::instance()->object_template(runner->isolate());
}

void ScriptController::UnhandledException(v8_glue::Runner*,
                                          const v8::TryCatch& try_catch) {
  base::string16 text;
  auto const error = try_catch.Exception();
  auto const message = try_catch.Message();
  if (!message.IsEmpty()) {
    text = base::StringPrintf(
            L"Exeption: %ls\n"
            L"Source: %ls\n"
            L"Source name: %ls(%d)\n",
            V8ToString(error).c_str(),
            V8ToString(message->GetSourceLine()).c_str(),
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
            V8ToString(frame->GetScriptName()).c_str(),
            frame->GetLineNumber());
      }
    }
  }  if (try_catch.HasTerminated()) {
    text = L"Scripe execution is terminated.";
  } else if (message.IsEmpty()) {
    text =  L"No details";
  }

  if (!started_) {
    ::MessageBoxW(nullptr, text.c_str(), L"Evita Startup Error", MB_ICONERROR);
    return;
  }

  auto const isolate = runner()->isolate();
  v8_glue::Runner::Scope runner_scope(runner());
  auto const console = runner_->GetGlobalProperty("console")->ToObject();
  auto const log = console->Get(gin::StringToV8(isolate, "log"));
  runner()->Call(log, console, gin::StringToV8(isolate, text));
}

}  // namespace dom
