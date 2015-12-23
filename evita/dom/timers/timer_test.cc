// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class TimerTest : public dom::AbstractDomTest {
 public:
  ~TimerTest() override = default;

 protected:
  TimerTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TimerTest);
};

TEST_F(TimerTest, OneShotTimer) {
  EXPECT_SCRIPT_VALID("var timer = new OneShotTimer();");
  EXPECT_SCRIPT_FALSE("timer.isRunning");
  EXPECT_SCRIPT_VALID(
      "var didFire = false;"
      "timer.start(0, function() { didFire = true; });");
  EXPECT_SCRIPT_TRUE("timer.isRunning");
  ::Sleep(0);
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("didFire");
  EXPECT_SCRIPT_FALSE("timer.isRunning");
}

TEST_F(TimerTest, RepeatingTimer) {
  EXPECT_SCRIPT_VALID("var timer = new RepeatingTimer();");
  EXPECT_SCRIPT_FALSE("timer.isRunning");
  EXPECT_SCRIPT_VALID(
      "var didFire = false;"
      "var count = 0;"
      "timer.start(1, function() {"
      "  ++count;"
      "  if (count == 2) this.stop();"
      "  didFire = true;"
      "});");
  EXPECT_SCRIPT_TRUE("timer.isRunning");
  ::Sleep(1);
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("didFire");
  EXPECT_SCRIPT_TRUE("timer.isRunning");
  ::Sleep(1);
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("timer.isRunning");
  EXPECT_SCRIPT_EQ("2", "count");
}

}  // namespace dom
