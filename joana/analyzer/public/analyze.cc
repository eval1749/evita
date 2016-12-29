// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyze.h"

#include "joana/analyzer/analyzer.h"

namespace joana {

using Analyzer = analyzer::Analyzer;

//
// AddExterns; the entry point
//
const ast::Node& AddExterns(AnalyzerSettings* context, const ast::Node& node) {
  Analyzer analyzer(context, node);
  return analyzer.AddExterns();
}

//
// Analyze; the entry point
//
const ast::Node& Analyze(AnalyzerSettings* context, const ast::Node& node) {
  Analyzer analyzer(context, node);
  return analyzer.Analyze();
}

}  // namespace joana
