// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/timing/mock_scheduler.h"

namespace dom {

class RequestIdleCallbackTest : public dom::AbstractDomTest {
 public:
  ~RequestIdleCallbackTest() override = default;

 protected:
  RequestIdleCallbackTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RequestIdleCallbackTest);
};

TEST_F(RequestIdleCallbackTest, cancelIdleCallback) {
  EXPECT_SCRIPT_VALID(
      "var didRun = false;"
      "function callback(idleDeadline) { didRun = true; }"
      "var taskId = Editor.requestIdleCallback(callback);"
      "Editor.cancelIdleCallback(taskId);");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("didRun");
}

TEST_F(RequestIdleCallbackTest, requestIdleCallback) {
  EXPECT_SCRIPT_VALID(
      "var didRun = false, didTimeout = false, timeRemaining = 0;"
      "function callback(idleDeadline) {"
      "  didRun = true;"
      "  didTimeout = idleDeadline.didTimeout;"
      "  timeRemaining = idleDeadline.timeRemaining();"
      "}"
      "Editor.requestIdleCallback(callback);");
  mock_scheduler()->SetIdleTimeRemaining(123);
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("didRun");
  EXPECT_SCRIPT_FALSE("didTimeout");
  EXPECT_SCRIPT_EQ("123", "timeRemaining");

  EXPECT_SCRIPT_VALID("didRun = false;");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("didRun") << "callback should not be in queue.";
}

TEST_F(RequestIdleCallbackTest, requestIdleCallbackTimeout) {
  EXPECT_SCRIPT_VALID(
      "var didRun = false, didTimeout = false, timeRemaining = 0;"
      "function callback(idleDeadline) {"
      "  didRun = true;"
      "  didTimeout = idleDeadline.didTimeout;"
      "  timeRemaining = idleDeadline.timeRemaining();"
      "}"
      "Editor.requestIdleCallback(callback, {timeout: 123});");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("didRun");
  mock_scheduler()->SetNowTicks(124);
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("didRun");
  EXPECT_SCRIPT_TRUE("didTimeout");
  EXPECT_SCRIPT_EQ("-1", "timeRemaining");

  EXPECT_SCRIPT_VALID("didRun = false;");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("didRun") << "callback should not be in queue.";
}

}  // namespace dom
