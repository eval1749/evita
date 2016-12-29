// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ANALYZE_H_
#define JOANA_ANALYZER_PUBLIC_ANALYZE_H_

#include <memory>

#include "base/macros.h"
#include "joana/analyzer/public/analyzer_export.h"

namespace joana {
namespace ast {
class Node;
}

namespace analyzer {
class Analyzer;
}  // namespace analyzer

class AnalyzerSettings;

//
// Analyzer provides public API of analyzer.
//
class JOANA_ANALYZER_EXPORT Analyzer final {
 public:
  explicit Analyzer(const AnalyzerSettings& settings);
  ~Analyzer();

  void Analyze();
  void Load(const ast::Node& node);

 private:
  std::unique_ptr<analyzer::Analyzer> analyzer_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ANALYZE_H_
