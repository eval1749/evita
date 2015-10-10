// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/script_host.h"

#include "base/command_line.h"
#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "gin/array_buffer.h"
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
#include "evita/v8_glue/v8_platform.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

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

void DidRejectPromise(v8::PromiseRejectMessage reject_message) {
  if (reject_message.GetEvent() != v8::kPromiseRejectWithNoHandler)
    return;
  auto const promise = reject_message.GetPromise();
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const js_console = runner->GetGlobalProperty("JsConsole");
  if (js_console.IsEmpty()) {
    DVLOG(0) << "No JsConsole";
    return;
  }
  auto const handler = js_console->ToObject()->Get(
      v8Strings::handleRejectedPromise.Get(isolate));
  if (handler.IsEmpty() || !handler->IsObject() ||
      !handler->ToObject()->IsFunction()) {
    DVLOG(0) << "No JsConsole.handleRejectedPromise";
    return;
  }
  runner->Call(handler, v8::Undefined(isolate), promise,
               reject_message.GetValue());
}

// Note: The constructor returned by v8::Object::GetConstructor() doesn't
// have properties defined in JavaScript.
v8::Handle<v8::Object> GetClassObject(v8::Isolate* isolate,
                                      v8::Handle<v8::Object> object) {
  auto const name = object->GetConstructorName();
  auto const value = isolate->GetCurrentContext()->Global()->Get(name);
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "No such class " << V8ToString(name) << ".";
    return v8::Handle<v8::Object>();
  }
  return value->ToObject();
}

void MessageBoxCallback(int count) {}

bool need_unlock_after_gc;
int suppress_message_box;

void MessageBox(const base::string16& message, int flags) {
  DVLOG(0) << message;
  if (suppress_message_box)
    return;
  domapi::Deferred<int, int> resolver;
  resolver.reject = base::Bind(MessageBoxCallback);
  resolver.resolve = base::Bind(MessageBoxCallback);
  ScriptHost::instance()->view_delegate()->MessageBox(
      kInvalidWindowId, message, L"Evita System Message", flags, resolver);
}

void GcEpilogueCallback(v8::GCType type, v8::GCCallbackFlags flags) {
  auto text =
      base::StringPrintf(L"GC finished type=%d flags=0x%X", type, flags);
  MessageBox(text, MB_ICONINFORMATION);
  if (need_unlock_after_gc)
    dom::Lock::instance()->Release(FROM_HERE);
}

void GcPrologueCallback(v8::GCType type, v8::GCCallbackFlags flags) {
  auto text = base::StringPrintf(L"GC Started type=%d flags=%X", type, flags);
  MessageBox(text, MB_ICONINFORMATION);
  need_unlock_after_gc = !dom::Lock::instance()->locked_by_dom();
  if (need_unlock_after_gc)
    dom::Lock::instance()->Acquire(FROM_HERE);
}

void MessageCallback(v8::Handle<v8::Message> message,
                     v8::Handle<v8::Value> error) {
  auto text = base::StringPrintf(
      L"Exception: %ls\n"
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
      text += base::StringPrintf(
          L"  at %ls (%ls(%d))\n", V8ToString(frame->GetFunctionName()).c_str(),
          V8ToString(frame->GetScriptName()).c_str(), frame->GetLineNumber());
    }
  }
  MessageBox(text, MB_ICONERROR);
}

void PopulateEnviromentStrings(v8_glue::Runner* runner) {
  auto const strings = ::GetEnvironmentStringsW();
  if (!strings)
    return;
  auto scanner = strings;
  while (*scanner) {
    while (*scanner) {
      ++scanner;
    }
    ++scanner;
  }
  --scanner;
  auto const isolate = runner->isolate();
  runner->global()
      ->Get(gin::StringToV8(isolate, "Os"))
      ->ToObject()
      ->Set(
          gin::StringToV8(isolate, "environmentStrings"),
          v8::String::NewFromTwoByte(
              isolate, reinterpret_cast<const uint16_t*>(strings),
              v8::String::kNormalString, static_cast<int>(scanner - strings)));
}

v8::Handle<v8::Object> ToMethodObject(v8::Isolate* isolate,
                                      v8::Handle<v8::Object> js_class,
                                      v8::Eternal<v8::String> method_name) {
  auto const value = js_class->Get(method_name.Get(isolate));
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "Object " << V8ToString(js_class) << " has no method '"
           << V8ToString(method_name.Get(isolate)) << "', it has "
           << V8ToString(js_class->GetPropertyNames()) << ".";
    return v8::Handle<v8::Object>();
  }
  return value->ToObject();
}

ScriptHost* script_host;

}  // namespace

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
      message_loop_for_script_(nullptr),
      state_(domapi::ScriptHostState::Stopped),
      testing_(false),
      testing_runner_(nullptr),
      view_delegate_(view_delegate) {
  DCHECK(!script_host);
  script_host = this;
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
  v8_glue::Runner::Scope runner_scope(runner);
  PopulateEnviromentStrings(runner);
}

ViewDelegate* ScriptHost::view_delegate() const {
  DCHECK(view_delegate_);
  return view_delegate_;
}

void ScriptHost::CallClassEventHandler(EventTarget* event_target,
                                       Event* event) {
  auto const isolate = runner()->isolate();
  auto const js_target = event_target->GetWrapper(isolate);
  auto const js_class = GetClassObject(isolate, js_target);
  if (js_class.IsEmpty())
    return;

  auto const js_method =
      ToMethodObject(isolate, js_class, v8Strings::handleEvent);
  if (js_method.IsEmpty())
    return;

  auto const js_event = event->GetWrapper(isolate);
  runner()->Call(js_method, js_target, js_event);
}

std::unique_ptr<ScriptHost> ScriptHost::Create(
    ViewDelegate* view_delegate,
    domapi::IoDelegate* io_delegate) {
  // See v8/src/flag-definitions.h
  // Note: |EnsureV8Initialized()| in "gin/isolate_holder.cc" also sets
  // flags.
  // char flags[] = "--use_strict" " --harmony";
  // v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
  gin::IsolateHolder::Initialize(gin::IsolateHolder::kStrictMode,
                                 gin::ArrayBufferAllocator::SharedInstance());
  v8::V8::InitializeICU();
  return std::unique_ptr<ScriptHost>(
      new ScriptHost(view_delegate, io_delegate));
}

void ScriptHost::DidStartScriptHost() {
  if (testing_)
    return;
  auto const script_sources = internal::GetJsLibSources();
  // We should prevent UI thread to access DOM.
  DOM_AUTO_LOCK_SCOPE();
  v8_glue::Runner::Scope runner_scope(runner());
  PopulateEnviromentStrings(runner());
  for (const auto& script_source : script_sources) {
    auto const result =
        runner()->Run(base::ASCIIToUTF16(script_source.script_text),
                      base::ASCIIToUTF16(script_source.file_name));
    if (result.IsEmpty()) {
      view_delegate_->DidStartScriptHost(state_);
      return;
    }
  }

  // Invoke |editors.start()| with command line arguments.
  auto const isolate = runner()->isolate();
  auto const js_editors = runner()->GetGlobalProperty("editors");
  auto const js_start =
      js_editors->ToObject()->Get(gin::StringToV8(isolate, "start"));
  if (!js_start->IsFunction()) {
    ThrowError("Editor.start isn't a function.");
    view_delegate_->DidStartScriptHost(state_);
    return;
  }
  auto const js_args =
      gin::ConvertToV8(runner()->context(),
                       base::CommandLine::ForCurrentProcess()->GetArgs())
          .ToLocalChecked();
  runner()->Call(js_start, js_editors, js_args);

  // Notify script execution completion to view.
  if (state_ == domapi::ScriptHostState::Stopped)
    state_ = domapi::ScriptHostState::Running;
  view_delegate_->DidStartScriptHost(state_);
}

void ScriptHost::PlatformError(const char* name) {
  auto const error = ::GetLastError();
  DVLOG(0) << "PlatformError " << name << " error=" << error;
  // TODO(yosi) Should be Win32Error.
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
  v8::TryCatch try_catch;
  DOM_AUTO_LOCK_SCOPE();
  runner()->isolate()->RunMicrotasks();
  runner()->HandleTryCatch(try_catch);
}

void ScriptHost::Start() {
  DCHECK(!message_loop_for_script_);
  message_loop_for_script_ = base::MessageLoop::current();

  // Node: Using editor::Application::instance() starts thread. So, we don't
  // start |ScriptHost| in testing. Although, we should remove
  // all |editor::Application::instance()| in DOM world.
  DCHECK(script_host);
  SuppressMessageBoxScope suppress_messagebox_scope;

  auto const isolate = this->isolate();
  auto const runner = new v8_glue::Runner(isolate, script_host);
  runner_.reset(runner);
  v8_glue::Runner::Scope runner_scope(runner);
  v8::V8::SetCaptureStackTraceForUncaughtExceptions(true);
  v8::V8::AddMessageListener(MessageCallback);
  v8::V8::AddGCPrologueCallback(GcPrologueCallback);
  v8::V8::AddGCEpilogueCallback(GcEpilogueCallback);
  // TODO(yosi) Turning off micro task running during creating wrapper, name
  // |Editor.checkSpelling('foo').then(console.log)| to work.
  // Otherwise |console.log| executed as micro task gets storage object which
  // doesn't have |v8_glue::WrapperInfo| at zeroth internal field.
  // See "985a73d2cce5", same thing is happened in spell checker with
  // |Editor.RegExp| object.
  isolate->SetAutorunMicrotasks(false);
  isolate->SetPromiseRejectCallback(DidRejectPromise);
  v8Strings::Init(isolate);
  DidStartScriptHost();
}

ScriptHost* ScriptHost::StartForTesting(ViewDelegate* view_delegate,
                                        domapi::IoDelegate* io_delegate) {
  if (!script_host) {
    script_host = ScriptHost::Create(view_delegate, io_delegate).release();
    script_host->testing_ = true;
    script_host->Start();
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
  v8_glue::Runner::Scope runner_scope(runner());
  auto exception = v8::Exception::Error(gin::StringToV8(isolate(), message));
  isolate()->ThrowException(exception);
}

void ScriptHost::ThrowRangeError(const std::string& message) {
  v8_glue::Runner::Scope runner_scope(runner());
  auto exception =
      v8::Exception::RangeError(gin::StringToV8(isolate(), message));
  isolate()->ThrowException(exception);
}

void ScriptHost::ThrowException(v8::Handle<v8::Value> exception) {
  v8_glue::Runner::Scope runner_scope(runner());
  isolate()->ThrowException(exception);
}

void ScriptHost::WillDestroyViewHost() {
  view_delegate_ = nullptr;
}

v8::Handle<v8::ObjectTemplate> ScriptHost::GetGlobalTemplate(
    v8_glue::Runner* runner) {
  return Global::instance()->object_template(runner->isolate());
}

void ScriptHost::UnhandledException(v8_glue::Runner*,
                                    const v8::TryCatch& try_catch) {
  ASSERT_DOM_LOCKED();
  base::string16 text;
  auto const error = try_catch.Exception();
  auto const message = try_catch.Message();
  if (!message.IsEmpty()) {
    text = base::StringPrintf(
        L"Exception: %ls\n"
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
                                   V8ToString(frame->GetScriptName()).c_str(),
                                   frame->GetLineNumber());
      }
    }
  } else if (try_catch.HasTerminated()) {
    text = L"Script execution is terminated.";
  } else if (message.IsEmpty()) {
    text = L"No details";
  }

  if (state_ != domapi::ScriptHostState::Running) {
    ::MessageBoxW(nullptr, text.c_str(), L"Evita Start up Error", MB_ICONERROR);
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
