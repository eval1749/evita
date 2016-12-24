// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANALYZER_H_
#define JOANA_ANALYZER_ANALYZER_H_

#include "base/macros.h"
#include "joana/analyzer/public/analyze.h"

namespace joana {
namespace analyzer {

class Analyzer final {
 public:
  Analyzer(AnalyzeContext* context, const ast::Node& node);
  ~Analyzer();

  const ast::Node& AddExterns();
  const ast::Node& Analyze();

 private:
  AnalyzeContext& context_;
  const ast::Node& root_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANALYZER_H_
