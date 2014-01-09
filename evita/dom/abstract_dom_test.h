// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_abstract_dom_test_h)
#define INCLUDE_evita_dom_abstract_dom_test_h

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"
#include "gtest/gtest.h"
#include "evita/v8_glue/v8.h"

namespace dom {

class MockViewImpl;
class ScriptController;
class ViewEventHandler;

class AbstractDomTest : public ::testing::Test {
  protected: typedef std::vector<v8::Handle<v8::Value>> Argv;

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

  protected: v8::Handle<v8::Value> Call(const base::StringPiece& name,
                                        const Argv& argv);
  protected: virtual void PopulateGlobalTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global_tempalte);
  protected: std::string RunScript(const std::string& text);
  protected: virtual void SetUp() override;
  protected: virtual void TearDown() override;

  DISALLOW_COPY_AND_ASSIGN(AbstractDomTest);
};
}  // namespace dom

#define EXPECT_SCRIPT_EQ(expect, script) EXPECT_EQ(expect, RunScript(script))
#define EXPECT_SCRIPT_FALSE(script) EXPECT_SCRIPT_EQ("false", (script))
#define EXPECT_SCRIPT_TRUE(script) EXPECT_SCRIPT_EQ("true", (script))

#endif //!defined(INCLUDE_evita_dom_abstract_dom_test_h)
