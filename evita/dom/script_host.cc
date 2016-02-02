// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>

#include "evita/dom/script_host.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/static_script_source.h"
#include "evita/dom/timing/idle_task.h"
#include "evita/dom/timing/performance.h"
#include "evita/dom/v8_strings.h"
#include "evita/dom/view_event_handler_impl.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_set.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/v8_platform.h"
#include "gin/array_buffer.h"

namespace dom {

namespace v8Strings {
void Init(v8::Isolate* isolate);
}

base::string16 V8ToString(v8::Local<v8::Value> value) {
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
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const js_editor = runner->GetGlobalProperty("Editor");
  if (js_editor.IsEmpty()) {
    DVLOG(0) << "No Editor";
    return;
  }
  auto const handler =
      js_editor->ToObject()->Get(v8Strings::handleRejectedPromise.Get(isolate));
  if (handler.IsEmpty() || !handler->IsObject() ||
      !handler->ToObject()->IsFunction()) {
    DVLOG(0) << "No Editor.handleRejectedPromise";
    return;
  }
  auto const event = static_cast<int>(reject_message.GetEvent());
  ASSERT_DOM_LOCKED();
  runner->CallAsFunction(handler, runner->global(), reject_message.GetPromise(),
                         reject_message.GetValue(),
                         gin::ConvertToV8(isolate, event));
}

// Note: The constructor returned by v8::Object::GetConstructor() doesn't
// have properties defined in JavaScript.
v8::Local<v8::Object> GetClassObject(v8::Isolate* isolate,
                                     v8::Local<v8::Object> object) {
  auto const name = object->GetConstructorName();
  auto const value = isolate->GetCurrentContext()->Global()->Get(name);
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "No such class " << V8ToString(name) << ".";
    return v8::Local<v8::Object>();
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
  domapi::Promise<int, int> resolver;
  resolver.reject = base::Bind(MessageBoxCallback);
  resolver.resolve = base::Bind(MessageBoxCallback);
  ScriptHost::instance()->view_delegate()->MessageBox(
      domapi::kInvalidWindowId, message, L"Evita System Message", flags,
      resolver);
}

const char* GcTypeToString(v8::GCType type) {
  static std::array<const char*, 5> strings{
      "Scavenge",     "MarkSweepCompact", "IncrementalMarking",
      "WeakCallback", "Unknown",
  };
  return strings[std::min(static_cast<size_t>(type), strings.size() - 1)];
}

void GcEpilogueCallback(v8::Isolate* isolate,
                        v8::GCType type,
                        v8::GCCallbackFlags flags) {
  TRACE_EVENT_END2("scheduler", "GcPrologueCallback", "type",
                   GcTypeToString(type), "flags", flags);
  auto text = base::StringPrintf("GC finished type=%s flags=0x%X",
                                 GcTypeToString(type), flags);
  MessageBox(base::ASCIIToUTF16(text), MB_ICONINFORMATION);
  if (need_unlock_after_gc)
    dom::Lock::instance()->Release(FROM_HERE);
}

void GcPrologueCallback(v8::Isolate* isolate,
                        v8::GCType type,
                        v8::GCCallbackFlags flags) {
  auto text = base::StringPrintf("GC Started type=%s flags=%X",
                                 GcTypeToString(type), flags);
  MessageBox(base::ASCIIToUTF16(text), MB_ICONINFORMATION);
  TRACE_EVENT_BEGIN2("scheduler", "GcPrologueCallback", "type",
                     GcTypeToString(type), "flags", flags);
  need_unlock_after_gc = !dom::Lock::instance()->locked_by_dom();
  if (!need_unlock_after_gc)
    return;
  dom::Lock::instance()->Acquire(FROM_HERE);
}

void MessageCallback(v8::Local<v8::Message> message,
                     v8::Local<v8::Value> error) {
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

v8::Local<v8::Object> ToMethodObject(v8::Isolate* isolate,
                                     v8::Local<v8::Object> js_class,
                                     v8::Eternal<v8::String> method_name) {
  auto const value = js_class->Get(method_name.Get(isolate));
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "Object " << V8ToString(js_class) << " has no method '"
           << V8ToString(method_name.Get(isolate)) << "', it has "
           << V8ToString(js_class->GetPropertyNames()) << ".";
    return v8::Local<v8::Object>();
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
ScriptHost::ScriptHost(Scheduler* scheduler,
                       domapi::ViewDelegate* view_delegate,
                       domapi::IoDelegate* io_delegate)
    : event_handler_(new ViewEventHandlerImpl(this)),
      io_delegate_(io_delegate),
      message_loop_for_script_(base::MessageLoop::current()),
      performance_(new Performance()),
      scheduler_(scheduler),
      state_(domapi::ScriptHostState::Stopped),
      testing_(false),
      testing_runner_(nullptr),
      view_delegate_(view_delegate) {}

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

Performance* ScriptHost::performance() const {
  return performance_.get();
}

v8_glue::Runner* ScriptHost::runner() const {
  DCHECK_EQ(message_loop_for_script_, base::MessageLoop::current());
  return testing_runner_ ? testing_runner_ : runner_.get();
}

void ScriptHost::set_testing_runner(v8_glue::Runner* runner) {
  testing_runner_ = runner;
  runner->set_user_data(this);
  v8_glue::Runner::Scope runner_scope(runner);
  PopulateEnviromentStrings(runner);
}

domapi::ViewDelegate* ScriptHost::view_delegate() const {
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
  runner()->CallAsFunction(js_method, js_target, js_event);
}

ScriptHost* ScriptHost::Create(Scheduler* scheduler,
                               domapi::ViewDelegate* view_delegate,
                               domapi::IoDelegate* io_delegate) {
  // See v8/src/flag-definitions.h
  // Note: |EnsureV8Initialized()| in "gin/isolate_holder.cc" also sets
  // flags.
  // char flags[] = "--use_strict" " --harmony";
  // v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
  // TODO(eval1749): Once v8/4659 fixed, we should enable
  // "--harmony-function_name".
  // See https://bugs.chromium.org/p/v8/issues/detail?id=4659
  char flags[] =
      // " --harmony-function-name"
      " --harmony-do-expressions";
  v8::V8::SetFlagsFromString(flags, sizeof(flags) - 1);
  gin::IsolateHolder::Initialize(gin::IsolateHolder::kStrictMode,
                                 gin::IsolateHolder::kStableV8Extras,
                                 gin::ArrayBufferAllocator::SharedInstance());
  v8::V8::InitializeICU();
  return new ScriptHost(scheduler, view_delegate, io_delegate);
}

void ScriptHost::CreateAndStart(Scheduler* scheduler,
                                domapi::ViewDelegate* view_delegate,
                                domapi::IoDelegate* io_delegate) {
  DCHECK(!script_host);
  script_host = Create(scheduler, view_delegate, io_delegate);
  script_host->Start();
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
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  auto const js_editors = runner()->GetGlobalProperty("editors");
  auto const js_start =
      js_editors->ToObject()->Get(gin::StringToV8(isolate, "start"));
  if (!js_start->IsFunction()) {
    ExceptionState exception_state(ExceptionState::Situation::MethodCall,
                                   runner()->context(), "global", "start");
    exception_state.ThrowError("Editor.start isn't a function.");
    view_delegate_->DidStartScriptHost(state_);
    return;
  }

  const auto& args = base::CommandLine::ForCurrentProcess()->GetArgs();
  const auto& js_args = gin::ConvertToV8(runner()->context(), args)
                            .FromMaybe(v8::Local<v8::Value>());
  if (js_args.IsEmpty()) {
    ExceptionState exception_state(ExceptionState::Situation::MethodCall,
                                   runner()->context(), "global", "start");
    exception_state.ThrowError("Failed to parse command line arguments");
    view_delegate_->DidStartScriptHost(state_);
    return;
  }
  runner()->CallAsFunction(js_start, js_editors, js_args);

  // Notify script execution completion to view.
  if (state_ == domapi::ScriptHostState::Stopped)
    state_ = domapi::ScriptHostState::Running;
  view_delegate_->DidStartScriptHost(state_);
}

void ScriptHost::ResetForTesting() {
  EditorWindow::ResetForTesting();
  Window::ResetForTesting();
}

// This function is called by |PromiseResolver::ScheduleRunMicrotasks|.
void ScriptHost::RunMicrotasks() {
  TRACE_EVENT0("script", "ScriptHost::RunMicrotasks");
  v8_glue::Runner::Scope runner_scope(runner());
  v8::TryCatch try_catch(runner()->isolate());
  try_catch.SetVerbose(true);
  DOM_AUTO_LOCK_SCOPE();
  runner()->isolate()->RunMicrotasks();
  runner()->HandleTryCatch(try_catch);
}

void ScriptHost::ScheduleIdleTask(const base::Closure& task) {
  scheduler_->ScheduleIdleTask(IdleTask(FROM_HERE, task));
}

void ScriptHost::Start() {
  // Node: Using editor::Application::instance() starts thread. So, we don't
  // start |ScriptHost| in testing. Although, we should remove
  // all |editor::Application::instance()| in DOM world.
  DCHECK(script_host);
  SuppressMessageBoxScope suppress_messagebox_scope;

  auto const isolate = this->isolate();
  auto const runner = new v8_glue::Runner(isolate, script_host);
  runner->set_user_data(this);
  runner_.reset(runner);
  v8_glue::Runner::Scope runner_scope(runner);
  isolate->SetCaptureStackTraceForUncaughtExceptions(true);
  isolate->AddMessageListener(MessageCallback);
  isolate->AddGCPrologueCallback(GcPrologueCallback);
  isolate->AddGCEpilogueCallback(GcEpilogueCallback);
  // TODO(eval1749): Turning off micro task running during creating wrapper,
  // name |Editor.checkSpelling('foo').then(console.log)| to work. Otherwise
  // |console.log| executed as micro task gets storage object which doesn't have
  // |v8_glue::WrapperInfo| at zeroth internal field. See "985a73d2cce5", same
  // thing is happened in spell checker with |Editor.RegExp| object.
  isolate->SetAutorunMicrotasks(false);
  isolate->SetPromiseRejectCallback(DidRejectPromise);
  v8Strings::Init(isolate);
  DidStartScriptHost();
}

ScriptHost* ScriptHost::StartForTesting(Scheduler* scheduler,
                                        domapi::ViewDelegate* view_delegate,
                                        domapi::IoDelegate* io_delegate) {
  if (!script_host) {
    script_host = ScriptHost::Create(scheduler, view_delegate, io_delegate);
    script_host->testing_ = true;
    script_host->Start();
  } else {
    // In testing, view_delegate is gmock'ed object. Each test case passes
    // newly constructed one.
    script_host->io_delegate_ = io_delegate;
    script_host->scheduler_ = scheduler;
    script_host->view_delegate_ = view_delegate;
    script_host->ResetForTesting();
  }
  suppress_message_box = 1;
  return script_host;
}

void ScriptHost::TerminateScriptExecution() {
  isolate()->TerminateExecution();
}

void ScriptHost::WillDestroyViewHost() {
  view_delegate_ = nullptr;
}

v8::Local<v8::ObjectTemplate> ScriptHost::GetGlobalTemplate(
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
  runner()->CallAsFunction(log, console, gin::StringToV8(isolate, text));
}

}  // namespace dom
