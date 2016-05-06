// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TESTING_GMOCK_H_
#define EVITA_DOM_TESTING_GMOCK_H_

#include "base/macros.h"
#include "v8/include/v8.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// GMock
//
class GMock final {
 public:
  static void Install(v8::Isolate* isolate,
                      v8::Local<v8::ObjectTemplate> global);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(GMock);
};

}  // namespace dom

#endif  // EVITA_DOM_TESTING_GMOCK_H_
