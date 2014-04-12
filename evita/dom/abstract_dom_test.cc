// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include <vector>

#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#pragma warning(pop)
#include "base/strings/utf_string_conversions.h"
#include "common/memory/singleton.h"
#include "evita/dom/converter.h"
#include "evita/dom/events/view_event_handler_impl.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/mock_io_delegate.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/static_script_source.h"

namespace dom {

using ::testing::_;

base::string16 V8ToString(v8::Handle<v8::Value> value);

namespace {
//////////////////////////////////////////////////////////////////////
//
// StaticScript
//
class StaticScript : public common::Singleton<StaticScript> {
  DECLARE_SINGLETON_CLASS(StaticScript);

  private: std::vector<v8::UniquePersistent<v8::UnboundScript>> unbound_scripts_;

  private: StaticScript();
  public: ~StaticScript();

  public: std::vector<v8::Local<v8::Script>> GetAll(v8::Isolate* isolate);
  private: void LoadAll(v8::Isolate* isolate);

  DISALLOW_COPY_AND_ASSIGN(StaticScript);
};

StaticScript::StaticScript() {
}

StaticScript::~StaticScript() {
}

std::vector<v8::Local<v8::Script>> StaticScript::GetAll(
    v8::Isolate* isolate) {
  if (unbound_scripts_.empty())
    LoadAll(isolate);
  std::vector<v8::Local<v8::Script>> scripts;
  for (auto& unbound_script : unbound_scripts_) {
    auto script = v8::Local<v8::UnboundScript>::New(isolate, unbound_script)->
        BindToCurrentContext();
    scripts.push_back(script);
  }
  return scripts;
}

void StaticScript::LoadAll(v8::Isolate* isolate) {
  for (const auto& script_source : internal::GetJsLibSources()) {
    v8::ScriptOrigin script_origin(
        gin::StringToV8(isolate, script_source.file_name)->ToString());
    v8::ScriptCompiler::Source source(
        gin::StringToV8(isolate, script_source.script_text),
        script_origin);
    auto unbound_script = v8::ScriptCompiler::CompileUnbound(isolate, &source);
    v8::UniquePersistent<v8::UnboundScript> handle(isolate, unbound_script);
    unbound_scripts_.push_back(std::move(handle));
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::RunnerScope
//
AbstractDomTest::RunnerScope::RunnerScope(AbstractDomTest* test)
    : runner_scope_(test->runner()) {
}

AbstractDomTest::RunnerScope::~RunnerScope() {
}


//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::ScriptCallArguments
//
AbstractDomTest::ScriptCallArguments::ScriptCallArguments(v8::Isolate* isolate)
    : isolate_(isolate) {
}

AbstractDomTest::ScriptCallArguments::~ScriptCallArguments() {
}

void AbstractDomTest::ScriptCallArguments::Populate() {
}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest
//
AbstractDomTest::AbstractDomTest()
      : mock_io_delegate_(new MockIoDelegate()),
        mock_view_impl_(new MockViewImpl()),
        script_host_(nullptr) {
}

AbstractDomTest::~AbstractDomTest() {
}

v8::Isolate* AbstractDomTest::isolate() const {
  return runner_->isolate();
}

domapi::ViewEventHandler* AbstractDomTest::view_event_handler() const {
  return script_host_->event_handler();
}

bool AbstractDomTest::DoCall(const base::StringPiece& name,
                             const Argv& argv) {
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  auto callee = runner_->GetGlobalProperty(name);
  auto const result = runner_->Call(callee, v8::Null(isolate), argv);
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
  v8::TryCatch try_catch;
  auto const script = v8::Script::Compile(
      gin::StringToV8(isolate, script_text), &script_origin);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  auto const result = script->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  return base::UTF16ToUTF8(V8ToString(result));
}

void AbstractDomTest::PopulateGlobalTemplate(v8::Isolate*,
                                             v8::Handle<v8::ObjectTemplate>) {
}

bool AbstractDomTest::RunScript(const base::StringPiece& script_text,
                                const char* file_name,
                                int line_number) {
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, line_number),
                                 v8::Integer::New(isolate, 0));
  v8::TryCatch try_catch;
  auto const script = v8::Script::Compile(
      gin::StringToV8(isolate, script_text), &script_origin);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  auto const result = script->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  v8::V8::RunMicrotasks(isolate);
  return true;
}

void AbstractDomTest::SetUp() {
  static int number_of_called;
  ++number_of_called;

  DEFINE_STATIC_LOCAL(base::MessageLoop, message_loop, ());

  EXPECT_CALL(*mock_view_impl_, RegisterViewEventHandler(_))
    .Times(number_of_called == 1 ? 1 : 0);

  script_host_ = dom::ScriptHost::StartForTesting(
    mock_view_impl_.get(), mock_io_delegate_.get());

  auto const isolate = script_host_->isolate();
  auto const runner = new v8_glue::Runner(isolate, this);
  runner_.reset(runner);
  ScriptHost::instance()->set_testing_runner(runner);
  v8_glue::Runner::Scope runner_scope(runner);
  for (auto script : StaticScript::instance()->GetAll(isolate)) {
    auto const result = runner->Run(script);
    if (result.IsEmpty())
      break;
  }
  ScriptHost::instance()->DidStartViewHost();
}

void AbstractDomTest::TearDown() {
  // Discard schedule tasks, e.g. DocumentSet::Observer callbacks.
  base::RunLoop run_loop;
  run_loop.RunUntilIdle();
}

namespace {
void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& info) {
  base::string16 message;
  for (int index = 0; index < info.Length(); ++index) {
    if (index)
      message += L" ";
    message += V8ToString(info[index]);
  }
  LOG(0) << message;
}
} // namespace

// v8_glue::RunnerDelegate
v8::Handle<v8::ObjectTemplate> AbstractDomTest::GetGlobalTemplate(
    v8_glue::Runner* runner) {
  DCHECK(!runner_.get());
  auto const templ =
    static_cast<v8_glue::RunnerDelegate*>(ScriptHost::instance())->
        GetGlobalTemplate(runner);

  auto const isolate = runner->isolate();
  v8_glue::RunnerDelegate temp_delegate;
  v8_glue::Runner temp_runner(isolate, &temp_delegate);
  auto const context = v8::Context::New(isolate);
  v8::Context::Scope context_scope(context);
  PopulateGlobalTemplate(isolate, templ);

  templ->Set(gin::StringToV8(isolate, "log"),
             v8::FunctionTemplate::New(isolate, LogCallback));

  return templ;
}

void AbstractDomTest::UnhandledException(v8_glue::Runner*,
                                         const v8::TryCatch& try_catch) {
  LOG(0) << V8ToString(try_catch.StackTrace());
  exception_ = base::UTF16ToUTF8(V8ToString(try_catch.Exception()));
}

}  // namespace dom

#include "base/strings/utf_string_conversions.h"
