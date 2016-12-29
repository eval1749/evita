// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANALYZER_H_
#define JOANA_ANALYZER_ANALYZER_H_

#include "base/macros.h"
#include "joana/analyzer/public/analyze.h"

namespace joana {
namespace analyzer {

//
// analyzer
//
class Analyzer final {
 public:
  explicit Analyzer(const AnalyzerSettings& settings);
  ~Analyzer();

  void Analyze();
  void Load(const ast::Node& node);

 private:
  const AnalyzerSettings& settings_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANALYZER_H_
