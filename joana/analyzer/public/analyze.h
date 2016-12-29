// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ANALYZE_H_
#define JOANA_ANALYZER_PUBLIC_ANALYZE_H_

#include "joana/analyzer/public/analyzer_export.h"

namespace joana {
namespace ast {
class Node;
}

class AnalyzerSettings;

//
// The analyzer entry point.
//
JOANA_ANALYZER_EXPORT const ast::Node& AddExterns(AnalyzerSettings* context,
                                                  const ast::Node& node);

JOANA_ANALYZER_EXPORT const ast::Node& Analyze(AnalyzerSettings* context,
                                               const ast::Node& node);

}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ANALYZE_H_
