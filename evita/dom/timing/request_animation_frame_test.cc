// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/timing/mock_scheduler.h"

namespace dom {

class RequestAnimationFrameTest : public dom::AbstractDomTest {
 public:
  ~RequestAnimationFrameTest() override = default;

 protected:
  RequestAnimationFrameTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RequestAnimationFrameTest);
};

TEST_F(RequestAnimationFrameTest, cancelAnimationFrame) {
  EXPECT_SCRIPT_VALID(
      "var didRun = false;"
      "function callback(time) { didRun = true; }"
      "var callbackId = Editor.requestAnimationFrame(callback);"
      "Editor.cancelAnimationFrame(callbackId);");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_FALSE("didRun");
}

TEST_F(RequestAnimationFrameTest, requestAnimationFrame) {
  EXPECT_SCRIPT_VALID(
      "var didRun = false;"
      "function callback(time) { didRun = true; }"
      "var callbackId = Editor.requestAnimationFrame(callback);");
  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("didRun");
}

}  // namespace dom
