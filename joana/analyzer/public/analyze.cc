// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyze.h"

#include "joana/analyzer/analyzer.h"

namespace joana {

Analyzer::Analyzer(const AnalyzerSettings& settings)
    : analyzer_(new analyzer::Analyzer(settings)) {}

Analyzer::~Analyzer() = default;

void Analyzer::Analyze() {
  analyzer_->Analyze();
}

void Analyzer::Load(const ast::Node& node) {
  analyzer_->Load(node);
}

}  // namespace joana
