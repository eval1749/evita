// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_host.h"

#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/text/document_set.h"
#include "evita/dom/events/view_event_handler_impl.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/static_script_source.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_set.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/per_isolate_data.h"
#include "evita/v8_glue/runner.h"

namespace dom {

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

  auto const window = WindowSet::instance()->Find(window_id);
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
  domapi::Deferred<int, int> resolver;
  resolver.reject = base::Bind(MessageBoxCallback);
  resolver.resolve = base::Bind(MessageBoxCallback);
  ScriptHost::instance()->view_delegate()->MessageBox(kInvalidWindowId,
    message, L"Evita System Message", flags, resolver);
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

ScriptHost* script_host;

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
// ScriptHost
//
ScriptHost::ScriptHost(ViewDelegate* view_delegate,
                                   domapi::IoDelegate* io_delegate)
    : event_handler_(new ViewEventHandlerImpl(this)),
      io_delegate_(io_delegate),
      message_loop_for_script_(base::MessageLoop::current()),
      state_(domapi::ScriptHostState::Stopped),
      testing_(false),
      testing_runner_(nullptr),
      view_delegate_(view_delegate) {
  view_delegate_->RegisterViewEventHandler(event_handler_.get());
  v8::V8::Initialize();
  v8::V8::InitializeICU();

  // See v8/src/flag-definitions.h
  // Note: |EnsureV8Initialized()| in "gin/isolate_holder.cc" also sets
  // flags.
  char flags[] =
      "--use_strict"
      " --harmony"
      " --harmony_typeof";
  v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
}

ScriptHost::~ScriptHost() {
  script_host = nullptr;
}

ScriptHost* ScriptHost::instance() {
  DCHECK(script_host);
  return script_host;
}

v8::Isolate* ScriptHost::isolate() const {
  return const_cast<ScriptHost*>(this)->isolate_holder_.isolate();
}

v8_glue::Runner* ScriptHost::runner() const {
  DCHECK_EQ(message_loop_for_script_, base::MessageLoop::current());
  return testing_runner_ ? testing_runner_ : runner_.get();
}

void ScriptHost::set_testing_runner(v8_glue::Runner* runner) {
  testing_runner_ = runner;
}

ViewDelegate* ScriptHost::view_delegate() const {
  DCHECK(view_delegate_);
  return view_delegate_;
}

void ScriptHost::DidStartViewHost() {
 if (testing_)
    return;
  auto const script_sources = internal::GetJsLibSources();
  // We should prevent UI thread to access DOM.
   DOM_AUTO_LOCK_SCOPE();
  v8_glue::Runner::Scope runner_scope(runner());
  for (const auto& script_source : script_sources) {
    auto const result = runner()->Run(
        base::ASCIIToUTF16(script_source.script_text),
        base::ASCIIToUTF16(script_source.file_name));
    if (result.IsEmpty()) {
      view_delegate_->DidStartScriptHost(state_);
      return;
    }
  }
  runner()->Run(L"editors.start([]);", L"__start__");
  if (state_ == domapi::ScriptHostState::Stopped)
    state_ = domapi::ScriptHostState::Running;
  view_delegate_->DidStartScriptHost(state_);
}

void ScriptHost::OpenFile(WindowId window_id,
                                const base::string16& file_name){
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
  v8::Handle<v8::Value> js_file_name = gin::StringToV8(isolate, file_name);
  DOM_AUTO_LOCK_SCOPE();
  runner()->Call(open_file, js_handler, js_file_name);
}

void ScriptHost::PlatformError(const char* name) {
  auto const error = ::GetLastError();
  DVLOG(0) << "PlatformError " << name << " erorr=" << error;
  // TODO(yosi) Shoulw be Win32Error.
  instance()->ThrowError(base::StringPrintf("%s error=%d", name, error));
}

void ScriptHost::PostTask(const tracked_objects::Location& from_here,
                                const base::Closure& task) {
  DCHECK_EQ(message_loop_for_script_, base::MessageLoop::current());
  message_loop_for_script_->PostTask(from_here, task);
}

void ScriptHost::ResetForTesting() {
  DocumentSet::instance()->ResetForTesting();
  EditorWindow::ResetForTesting();
  Window::ResetForTesting();
}

// This function is called by |PromiseResolver::ScheduleRunMicrotasks|.
void ScriptHost::RunMicrotasks() {
  v8_glue::Runner::Scope runner_scope(runner());
  DOM_AUTO_LOCK_SCOPE();
  runner()->isolate()->RunMicrotasks();
}

ScriptHost* ScriptHost::Start(ViewDelegate* view_delegate,
                                          domapi::IoDelegate* io_deleage) {
  // Node: Useing Application::instance() starts thread. So, we don't
  // start |ScriptHost| in testing. Although, we should remove
  // all |Application::instance()| in DOM world.
  if (script_host && script_host->testing_)
    return script_host;
  DCHECK(!script_host);
  SuppressMessageBoxScope suppress_messagebox_scope;
  script_host = new ScriptHost(view_delegate, io_deleage);
  auto const isolate = script_host->isolate();
  auto const runner = new v8_glue::Runner(isolate, script_host);
  script_host->runner_.reset(runner);
  v8_glue::Runner::Scope runner_scope(runner);
  v8::V8::SetCaptureStackTraceForUncaughtExceptions(true);
  v8::V8::AddMessageListener(MessageCallback);
  v8::V8::AddGCPrologueCallback(GcPrologueCallback);
  v8::V8::AddGCEpilogueCallback(GcEpilogueCallback);
  // TODO(yosi) Turning off micro task runining during creating wrapper, mame
  // |Editor.checkSpelling('foo').then(console.log)| to work.
  // Othewise |console.log| executed as micro task gets storage object which
  // doesn't have |v8_glue::WrapperInfo| at zeroth internal field.
  // See "985a73d2cce5", same thing is happened in spell checker with
  // |Editor.RegExp| object.
  // Note: We run micro tasks in |ViewEventHandlerImpl::DispatchViewIdleEvent|.
  isolate->SetAutorunMicrotasks(false);
  v8Strings::Init(isolate);
  return script_host;
}

ScriptHost* ScriptHost::StartForTesting(
    ViewDelegate* view_delegate, domapi::IoDelegate* io_delegate) {
  if (!script_host) {
    Start(view_delegate, io_delegate)->testing_ = true;
  } else {
    // In testing, view_delegate is gmock'ed object. Each test case passes
    // newly constructed one.
    script_host->io_delegate_ = io_delegate;
    script_host->view_delegate_ = view_delegate;
    script_host->ResetForTesting();
  }
  suppress_message_box = 1;
  return script_host;
}

void ScriptHost::ThrowError(const std::string& message) {
  auto isolate = isolate_holder_.isolate();
  isolate->ThrowException(v8::Exception::Error(
      gin::StringToV8(isolate, message)));
}

void ScriptHost::ThrowException(v8::Handle<v8::Value> exception) {
  auto isolate = isolate_holder_.isolate();
  isolate->ThrowException(exception);
}

void ScriptHost::WillDestroyHost() {
  view_delegate_ = nullptr;
}

v8::Handle<v8::ObjectTemplate> ScriptHost::GetGlobalTemplate(
    v8_glue::Runner* runner) {
  return Global::instance()->object_template(runner->isolate());
}

void ScriptHost::UnhandledException(v8_glue::Runner*,
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

  if (state_ != domapi::ScriptHostState::Running) {
    ::MessageBoxW(nullptr, text.c_str(), L"Evita Startup Error", MB_ICONERROR);
    state_ = domapi::ScriptHostState::Error;
    return;
  }

  auto const isolate = runner()->isolate();
  DVLOG(0) << text;
  v8_glue::Runner::Scope runner_scope(runner());
  auto const js_console = runner_->GetGlobalProperty("console");
  if (js_console.IsEmpty() || !js_console->IsObject()) {
    DVLOG(0) << "No console object. Why?";
    return;
  }
  auto const console = js_console->ToObject();
  auto const log = console->Get(gin::StringToV8(isolate, "log"));
  runner()->Call(log, console, gin::StringToV8(isolate, text));
}

}  // namespace dom
