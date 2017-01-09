// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PASS_H_
#define JOANA_ANALYZER_PASS_H_

#include "joana/analyzer/context_user.h"

namespace joana {

namespace ast {
class Node;
}

class SourceCodeRange;

namespace analyzer {

class Context;
enum class ErrorCode;
class Factory;

//
// Pass
//
class Pass : public ContextUser {
 public:
  ~Pass();

 protected:
  explicit Pass(Context* context);

 private:
  DISALLOW_COPY_AND_ASSIGN(Pass);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PASS_H_
