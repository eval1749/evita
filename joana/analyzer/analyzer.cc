// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/analyzer.h"

namespace joana {
namespace analyzer {

Analyzer::Analyzer(const AnalyzerSettings& settings) : settings_(settings) {}

Analyzer::~Analyzer() = default;

void Analyzer::Analyze() {}

void Analyzer::Load(const ast::Node& node) {}

}  // namespace analyzer
}  // namespace joana
