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
#include "evita/v8_glue/script_callback.h"
#include "evita/v8_glue/v8.h"

namespace dom {

class MockViewImpl;
class ScriptController;
class ViewEventHandler;

class AbstractDomTest : public ::testing::Test {
  protected: typedef v8_glue::internal::ScriptClosure::Argv Argv;

  private: v8::UniquePersistent<v8::Context> context_;
  private: std::unique_ptr<MockViewImpl> mock_view_impl_;

  // Note: ScriptController is a singleton.
  private: ScriptController* script_controller_;

  protected: AbstractDomTest();
  protected: virtual ~AbstractDomTest();

  protected: v8::Isolate* isolate() const;
  protected: MockViewImpl* mock_view_impl() const {
    return mock_view_impl_.get();
  }
  protected: ViewEventHandler* view_event_handler() const;

  protected: template<typename... Params>
      bool Call(const base::StringPiece& name, Params... params);
  private: bool DoCall(v8::Isolate* isolate, const base::StringPiece& name,
                       const Argv& argv);
  protected: std::string EvalScript(const std::string& text);
  protected: virtual void PopulateGlobalTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global_tempalte);
  protected: bool RunScript(const std::string& text);
  protected: virtual void SetUp() override;
  protected: virtual void TearDown() override;

  DISALLOW_COPY_AND_ASSIGN(AbstractDomTest);
};

template<typename... Params>
bool AbstractDomTest::Call(const base::StringPiece& name, Params... params) {
  v8::HandleScope handle_scope(isolate());
  v8_glue::internal::ScriptCallbackArguments args(isolate());
  args.Populate(params...);
  return DoCall(isolate(), name, args.argv());
}

}  // namespace dom

#define EXPECT_VALID_SCRIPT(script) EXPECT_TRUE(RunScript(script))
#define EXPECT_VALID_SCRIPT_CALL(name, ...) \
    EXPECT_TRUE(Call(name, __VA_ARGS__))
#define EXPECT_SCRIPT_EQ(expect, script) EXPECT_EQ(expect, EvalScript(script))
#define EXPECT_SCRIPT_FALSE(script) EXPECT_SCRIPT_EQ("false", (script))
#define EXPECT_SCRIPT_TRUE(script) EXPECT_SCRIPT_EQ("true", (script))

#endif //!defined(INCLUDE_evita_dom_abstract_dom_test_h)
