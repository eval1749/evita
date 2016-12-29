// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyze.h"

#include "joana/analyzer/controller.h"

namespace joana {

Analyzer::Analyzer(const AnalyzerSettings& settings)
    : controller_(new analyzer::Controller(settings)) {}

Analyzer::~Analyzer() = default;

void Analyzer::Analyze() {
  controller_->Analyze();
}

void Analyzer::Load(const ast::Node& node) {
  controller_->Load(node);
}

}  // namespace joana
