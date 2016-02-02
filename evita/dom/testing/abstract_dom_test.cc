// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include <vector>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "common/memory/singleton.h"
#include "evita/dom/converter.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/static_script_source.h"
#include "evita/dom/testing/gtest.h"
#include "evita/dom/testing/mock_io_delegate.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "evita/dom/timing/mock_scheduler.h"
#include "evita/dom/view_event_handler_impl.h"
#include "evita/v8_glue/runner_delegate.h"

namespace dom {

using ::testing::_;

base::string16 V8ToString(v8::Local<v8::Value> value);

namespace {
v8_glue::Runner* static_runner;

//////////////////////////////////////////////////////////////////////
//
// StaticScript
//
class StaticScript : public common::Singleton<StaticScript> {
  DECLARE_SINGLETON_CLASS(StaticScript);

 public:
  ~StaticScript();

  std::vector<v8::Local<v8::Script>> GetAll(v8::Isolate* isolate);

 private:
  StaticScript();

  void LoadAll(v8::Isolate* isolate);

  std::vector<v8::UniquePersistent<v8::UnboundScript>> unbound_scripts_;

  DISALLOW_COPY_AND_ASSIGN(StaticScript);
};

StaticScript::StaticScript() {}

StaticScript::~StaticScript() {}

std::vector<v8::Local<v8::Script>> StaticScript::GetAll(v8::Isolate* isolate) {
  if (unbound_scripts_.empty())
    LoadAll(isolate);
  std::vector<v8::Local<v8::Script>> scripts;
  for (auto& unbound_script : unbound_scripts_) {
    auto script = v8::Local<v8::UnboundScript>::New(isolate, unbound_script)
                      ->BindToCurrentContext();
    scripts.push_back(script);
  }
  return scripts;
}

void StaticScript::LoadAll(v8::Isolate* isolate) {
  for (const auto& script_source : internal::GetJsLibSources()) {
    v8::ScriptOrigin script_origin(
        gin::StringToV8(isolate, script_source.file_name)->ToString());
    v8::ScriptCompiler::Source source(
        gin::StringToV8(isolate, script_source.script_text), script_origin);
    auto unbound_script =
        v8::ScriptCompiler::CompileUnboundScript(isolate, &source)
            .ToLocalChecked();
    v8::UniquePersistent<v8::UnboundScript> handle(isolate, unbound_script);
    unbound_scripts_.push_back(std::move(handle));
  }
}

class RunnerDelegateMock final : public v8_glue::RunnerDelegate {
 public:
  RunnerDelegateMock() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RunnerDelegateMock);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::RunnerDelegate
//
class AbstractDomTest::RunnerDelegate final
    : public common::Singleton<RunnerDelegate>,
      public v8_glue::RunnerDelegate {
 public:
  RunnerDelegate() = default;
  ~RunnerDelegate() final = default;

  void set_test_instance(AbstractDomTest* test_instance) {
    test_instance_ = test_instance;
  }

 private:
  // v8_glue::RunnerDelegate
  v8::Local<v8::ObjectTemplate> GetGlobalTemplate(
      v8_glue::Runner* runner) final;
  void UnhandledException(v8_glue::Runner* runner,
                          const v8::TryCatch& try_catch) final;

  AbstractDomTest* test_instance_;
};

// v8_glue::RunnerDelegate
v8::Local<v8::ObjectTemplate>
AbstractDomTest::RunnerDelegate::GetGlobalTemplate(v8_glue::Runner* runner) {
  auto const templ =
      static_cast<v8_glue::RunnerDelegate*>(test_instance_->script_host_)
          ->GetGlobalTemplate(runner);
  auto const isolate = runner->isolate();
  RunnerDelegateMock temp_delegate;
  v8_glue::Runner temp_runner(isolate, &temp_delegate);
  auto const context = v8::Context::New(isolate);
  v8::Context::Scope context_scope(context);
  test_instance_->PopulateGlobalTemplate(isolate, templ);
  GTest::Install(isolate, templ);
  return templ;
}

void AbstractDomTest::RunnerDelegate::UnhandledException(
    v8_glue::Runner*,
    const v8::TryCatch& try_catch) {
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
  LOG(0) << text;
  test_instance_->exception_ =
      base::UTF16ToUTF8(V8ToString(try_catch.Exception()));
}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::RunnerScope
//
AbstractDomTest::RunnerScope::RunnerScope(AbstractDomTest* test)
    : runner_scope_(test->runner()) {}

AbstractDomTest::RunnerScope::~RunnerScope() {}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::ScriptCallArguments
//
AbstractDomTest::ScriptCallArguments::ScriptCallArguments(v8::Isolate* isolate)
    : isolate_(isolate) {}

AbstractDomTest::ScriptCallArguments::~ScriptCallArguments() {}

void AbstractDomTest::ScriptCallArguments::Populate() {}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest
//
AbstractDomTest::AbstractDomTest()
    : mock_io_delegate_(new MockIoDelegate()),
      mock_scheduler_(new MockScheduler()),
      mock_view_impl_(new MockViewImpl()),
      script_host_(nullptr) {}

AbstractDomTest::~AbstractDomTest() {}

v8::Isolate* AbstractDomTest::isolate() const {
  return runner_->isolate();
}

domapi::ViewEventHandler* AbstractDomTest::view_event_handler() const {
  return script_host_->event_handler();
}

base::FilePath AbstractDomTest::BuildPath(
    const std::vector<base::StringPiece>& components) {
  base::FilePath path;
  PathService::Get(base::DIR_SOURCE_ROOT, &path);
  path = path.AppendASCII("src").AppendASCII("evita").AppendASCII("dom");
  for (const auto& component : components)
    path = path.AppendASCII(component);
  return path;
}

bool AbstractDomTest::DoCall(const base::StringPiece& name, const Argv& argv) {
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  auto callee = runner_->GetGlobalProperty(name);
  auto const result = runner_->CallAsFunction(callee, v8::Null(isolate), argv);
  return !result.IsEmpty();
}

std::string AbstractDomTest::EvalScript(const base::StringPiece& script_text,
                                        const char* file_name,
                                        int line_number) {
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, line_number),
                                 v8::Integer::New(isolate, 0));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  auto script = v8::ScriptCompiler::Compile(runner_->context(), &source);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  DOM_AUTO_LOCK_SCOPE();
  auto const result = script.ToLocalChecked()->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  return base::UTF16ToUTF8(V8ToString(result));
}

void AbstractDomTest::PopulateGlobalTemplate(v8::Isolate*,
                                             v8::Local<v8::ObjectTemplate>) {}

void AbstractDomTest::RunFile(const base::FilePath& path) {
  std::string source;
  ASSERT_TRUE(base::ReadFileToString(path, &source)) << path.LossyDisplayName();
  ASSERT_TRUE(RunScript(source, base::UTF16ToUTF8(path.LossyDisplayName()), 0));
}

void AbstractDomTest::RunFile(
    const std::vector<base::StringPiece>& components) {
  RunFile(BuildPath(components));
}

bool AbstractDomTest::RunScript(const base::StringPiece& script_text,
                                const base::StringPiece& file_name,
                                int line_number) {
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, line_number),
                                 v8::Integer::New(isolate, 0));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  auto script = v8::ScriptCompiler::Compile(runner_->context(), &source);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  DOM_AUTO_LOCK_SCOPE();
  auto const result = script.ToLocalChecked()->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  isolate->RunMicrotasks();
  return true;
}

void AbstractDomTest::RunMessageLoopUntilIdle() {
  base::RunLoop run_loop;
  run_loop.RunUntilIdle();
  mock_scheduler_->RunPendingTasks();
  runner_->isolate()->RunMicrotasks();
}

void AbstractDomTest::SetUp() {
  CR_DEFINE_STATIC_LOCAL(base::MessageLoop, message_loop, ());

  RunnerDelegate::instance()->set_test_instance(this);

  script_host_ = dom::ScriptHost::StartForTesting(
      mock_scheduler_.get(), mock_view_impl_.get(), mock_io_delegate_.get());

  auto const isolate = script_host_->isolate();

  if (static_runner) {
    script_host_->set_testing_runner(static_runner);
    runner_.reset(static_runner);

    // Install test specific JavaScript objects.
    RunnerScope runner_scope(this);
    auto const object_template = v8::ObjectTemplate::New(isolate);
    PopulateGlobalTemplate(isolate, object_template);
    auto const global = runner_->global();
    auto const object = object_template->NewInstance();
    auto const keys = object->GetPropertyNames();
    auto const keys_length = keys->Length();
    for (auto index = 0u; index < keys_length; ++index) {
      auto const key = keys->Get(index);
      global->Set(key, object->Get(key));
    }
    runner_->Run(L"initialize()", L"*test*");
    return;
  }

  auto const runner = new v8_glue::Runner(isolate, RunnerDelegate::instance());
  runner_.reset(runner);
  script_host_->set_testing_runner(runner);

  // Load library files.
  v8_glue::Runner::Scope runner_scope(runner);
  for (const auto& script : StaticScript::instance()->GetAll(isolate)) {
    const auto& unbound_script = script->GetUnboundScript();
    DVLOG(0) << "Run "
             << V8ToString(unbound_script->GetScriptName()->ToString());
    auto const result = runner->Run(script);
    if (result.IsEmpty())
      break;
  }
}

void AbstractDomTest::TearDown() {
  // Discard schedule tasks, e.g. TextDocumentSet::Observer callbacks.
  base::RunLoop run_loop;
  run_loop.RunUntilIdle();
  if (!static_runner || static_runner == runner_.get())
    static_runner = runner_.release();
}

void AbstractDomTest::UnhandledException(v8_glue::Runner* runner,
                                         const v8::TryCatch& try_catch) {
  static_cast<v8_glue::RunnerDelegate*>(RunnerDelegate::instance())
      ->UnhandledException(runner, try_catch);
}

}  // namespace dom
