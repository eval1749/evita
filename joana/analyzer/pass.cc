// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/pass.h"

#include "joana/analyzer/context.h"
#include "joana/ast/node.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"

namespace joana {
namespace analyzer {

//
// Pass
//
Pass::Pass(Context* context) : context_(*context) {}
Pass::~Pass() = default;

Factory& Pass::factory() {
  return context_.factory();
}

void Pass::AddError(const ast::Node& node,
                    ErrorCode error_code,
                    const ast::Node& related) {
  if (node.range().source_code() == related.range().source_code()) {
    AddError(SourceCodeRange::Merge(node.range(), related.range()), error_code);
    return;
  }
  AddError(related, error_code);
  AddError(node, error_code);
}

void Pass::AddError(const ast::Node& node, ErrorCode error_code) {
  AddError(node.range(), error_code);
}

void Pass::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  context_.error_sink().AddError(range, error_code);
}

}  // namespace analyzer
}  // namespace joana
