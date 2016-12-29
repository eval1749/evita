// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/analyzer.h"

namespace joana {
namespace analyzer {

Analyzer::Analyzer(AnalyzerSettings* context, const ast::Node& root)
    : context_(*context), root_(root) {}

Analyzer::~Analyzer() = default;

const ast::Node& Analyzer::AddExterns() {
  return root_;
}

const ast::Node& Analyzer::Analyze() {
  return root_;
}

}  // namespace analyzer
}  // namespace joana
