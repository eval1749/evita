// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_abstract_dom_test_h)
#define INCLUDE_evita_dom_abstract_dom_test_h

#include <memory>
#include <string>

#include "base/strings/string_piece.h"
#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)
#include "evita/v8_glue/runner_delegate.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "evita/v8_glue/v8.h"

namespace domapi {
class ViewEventHandler;
}

namespace dom {

class MockIoDelegate;
class MockViewImpl;
class ScriptHost;

class AbstractDomTest : public v8_glue::RunnerDelegate,
                        public ::testing::Test {
  protected: typedef v8_glue::internal::ScriptClosure::Argv Argv;

  protected: class RunnerScope {
    private: v8_glue::Runner::Scope runner_scope_;
    public: RunnerScope(AbstractDomTest* test);
    public: ~RunnerScope();
  };

  private: std::string exception_;
  private: std::unique_ptr<MockIoDelegate> mock_io_delegate_;
  private: std::unique_ptr<MockViewImpl> mock_view_impl_;
  private: std::unique_ptr<v8_glue::Runner> runner_;

  // Note: ScriptHost is a singleton.
  private: ScriptHost* script_host_;

  protected: AbstractDomTest();
  protected: virtual ~AbstractDomTest();

  protected: v8::Isolate* isolate() const;
  protected: MockIoDelegate* mock_io_delegate() const {
    return mock_io_delegate_.get();
  }
  protected: MockViewImpl* mock_view_impl() const {
    return mock_view_impl_.get();
  }
  protected: v8_glue::Runner* runner() const { return runner_.get(); }
  protected: domapi::ViewEventHandler* view_event_handler() const;

  protected: template<typename... Params>
      bool Call(const base::StringPiece& name, Params... params);
  private: bool DoCall(const base::StringPiece& name, const Argv& argv);
  protected: std::string EvalScript(const base::StringPiece& text,
                                    const char* file_name, int line_number);
  protected: virtual void PopulateGlobalTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global_tempalte);
  protected: bool RunScript(const base::StringPiece& text,
                            const char* file_name, int line_number);
  protected: virtual void SetUp() override;
  protected: virtual void TearDown() override;

  // v8_glue::RunnerDelegate
  private: virtual v8::Handle<v8::ObjectTemplate>
      GetGlobalTemplate(v8_glue::Runner* runner) override;
  private: virtual void UnhandledException(v8_glue::Runner* runner,
      const v8::TryCatch& try_catch) override;

  DISALLOW_COPY_AND_ASSIGN(AbstractDomTest);
};

template<typename... Params>
bool AbstractDomTest::Call(const base::StringPiece& name, Params... params) {
  v8::HandleScope handle_scope(isolate());
  v8_glue::internal::ScriptCallbackArguments args(isolate());
  args.Populate(params...);
  return DoCall(name, args.argv());
}

}  // namespace dom

#define EXPECT_SCRIPT_VALID(script) \
    EXPECT_TRUE(RunScript(script, __FILE__, __LINE__))
#define EXPECT_SCRIPT_VALID_CALL(name, ...) \
    EXPECT_TRUE(Call(name, __VA_ARGS__))
#define EXPECT_SCRIPT_EQ(expect, script) \
    EXPECT_EQ(expect, EvalScript(script, __FILE__, __LINE__))
#define EXPECT_SCRIPT_FALSE(script) EXPECT_SCRIPT_EQ("false", (script))
#define EXPECT_SCRIPT_TRUE(script) EXPECT_SCRIPT_EQ("true", (script))

#endif //!defined(INCLUDE_evita_dom_abstract_dom_test_h)
