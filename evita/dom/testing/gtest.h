// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TESTING_GTEST_H_
#define EVITA_DOM_TESTING_GTEST_H_

#include "v8/include/v8.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// GTest
//
class GTest final {
 public:
  GTest() = delete;
  ~GTest() = delete;

  static void Install(v8::Isolate* isolate,
                      v8::Local<v8::ObjectTemplate> global);
};

}  // namespace dom

#endif  // EVITA_DOM_TESTING_GTEST_H_
