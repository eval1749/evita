// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/analyzer.h"

namespace joana {
namespace internal {

Analyzer::Analyzer(AnalyzeContext* context, const ast::Node& root)
    : context_(*context), root_(root) {}

Analyzer::~Analyzer() = default;

const ast::Node& Analyzer::Run() {
  return root_;
}

}  // namespace internal
}  // namespace joana
