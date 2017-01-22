// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/pass.h"

#include "joana/analyzer/context.h"
#include "joana/ast/node.h"
#include "joana/base/error_sink.h"

namespace joana {
namespace analyzer {

//
// Pass
//
Pass::Pass(Context* context) : ContextUser(context) {}
Pass::~Pass() = default;

void Pass::RunOnAll() {}

}  // namespace analyzer
}  // namespace joana
