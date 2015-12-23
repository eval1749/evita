// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TESTING_ABSTRACT_DOM_TEST_H_
#define EVITA_DOM_TESTING_ABSTRACT_DOM_TEST_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"
#include "gtest/gtest.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/v8.h"

namespace base {
class FilePath;
}

namespace domapi {
class ViewEventHandler;
}

namespace dom {

class MockIoDelegate;
class MockScheduler;
class MockViewImpl;
class ScriptHost;

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest - The base class for testing JavaScript.
//
class AbstractDomTest : public ::testing::Test {
 protected:
  typedef std::vector<v8::Handle<v8::Value>> Argv;

  class RunnerScope final {
   public:
    explicit RunnerScope(AbstractDomTest* test);
    ~RunnerScope();

   private:
    v8_glue::Runner::Scope runner_scope_;

    DISALLOW_COPY_AND_ASSIGN(RunnerScope);
  };

  class ScriptCallArguments final {
   public:
    explicit ScriptCallArguments(v8::Isolate* isolate);
    ~ScriptCallArguments();

    const Argv& argv() const { return argv_; }

    void Populate();

    template <typename Arg1, typename... Args>
    void Populate(const Arg1& arg1, const Args&... args) {
      argv_.push_back(gin::Converter<Arg1>::ToV8(isolate_, arg1));
      Populate(args...);
    }

   private:
    Argv argv_;
    v8::Isolate* isolate_;

    DISALLOW_COPY_AND_ASSIGN(ScriptCallArguments);
  };

  AbstractDomTest();
  ~AbstractDomTest() override;

  v8::Isolate* isolate() const;
  MockIoDelegate* mock_io_delegate() const { return mock_io_delegate_.get(); }
  MockScheduler* mock_scheduler() const { return mock_scheduler_.get(); }
  MockViewImpl* mock_view_impl() const { return mock_view_impl_.get(); }
  v8_glue::Runner* runner() const { return runner_.get(); }
  domapi::ViewEventHandler* view_event_handler() const;

  template <typename... Params>
  bool Call(const base::StringPiece& name, Params... params);
  bool DoCall(const base::StringPiece& name, const Argv& argv);
  std::string EvalScript(const base::StringPiece& text,
                         const char* file_name,
                         int line_number);
  virtual void PopulateGlobalTemplate(
      v8::Isolate* isolate,
      v8::Handle<v8::ObjectTemplate> global_tempalte);

  void RunFile(const base::FilePath& path);
  void RunFile(const std::vector<base::StringPiece>& components);

  // When test case posts a task to message loop, it should call
  // |RunMessageLoopUntilIdle()| to run posted tasks.
  void RunMessageLoopUntilIdle();

  bool RunScript(const base::StringPiece& text,
                 const base::StringPiece& file_name,
                 int line_number);
  void SetUp() override;
  void TearDown() override;

 private:
  class RunnerDelegate;
  friend class RunnerDelegate;

  base::FilePath BuildPath(const std::vector<base::StringPiece>& components);
  void UnhandledException(v8_glue::Runner* runner,
                          const v8::TryCatch& try_catch);

  std::string exception_;
  const std::unique_ptr<MockIoDelegate> mock_io_delegate_;
  const std::unique_ptr<MockScheduler> mock_scheduler_;
  const std::unique_ptr<MockViewImpl> mock_view_impl_;
  std::unique_ptr<v8_glue::Runner> runner_;

  // Note: ScriptHost is a singleton.
  ScriptHost* script_host_;

  DISALLOW_COPY_AND_ASSIGN(AbstractDomTest);
};

template <typename... Params>
bool AbstractDomTest::Call(const base::StringPiece& name, Params... params) {
  v8::HandleScope handle_scope(isolate());
  ScriptCallArguments args(isolate());
  args.Populate(params...);
  return DoCall(name, args.argv());
}

}  // namespace dom

#define EXPECT_SCRIPT_VALID(script) \
  EXPECT_TRUE(RunScript(script, __FILE__, __LINE__))
#define EXPECT_SCRIPT_VALID_CALL(name, ...) EXPECT_TRUE(Call(name, __VA_ARGS__))
#define EXPECT_SCRIPT_EQ(expect, script) \
  EXPECT_EQ(expect, EvalScript(script, __FILE__, __LINE__))
#define EXPECT_SCRIPT_FALSE(script) EXPECT_SCRIPT_EQ("false", (script))
#define EXPECT_SCRIPT_TRUE(script) EXPECT_SCRIPT_EQ("true", (script))

#endif  // EVITA_DOM_TESTING_ABSTRACT_DOM_TEST_H_
