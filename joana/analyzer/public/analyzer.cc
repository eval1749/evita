// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyzer.h"

#include "joana/analyzer/controller.h"

namespace joana {

Analyzer::Analyzer(const AnalyzerSettings& settings)
    : controller_(new analyzer::Controller(settings)) {}

Analyzer::~Analyzer() = default;

const ast::Node& Analyzer::built_in_module() const {
  return controller_->built_in_module();
}

void Analyzer::Analyze() {
  controller_->Analyze();
}

void Analyzer::Load(const ast::Node& node) {
  controller_->Load(node);
}

}  // namespace joana
