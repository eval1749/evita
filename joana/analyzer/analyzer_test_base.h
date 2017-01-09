// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANALYZER_TEST_BASE_H_
#define JOANA_ANALYZER_ANALYZER_TEST_BASE_H_

#include <memory>

#include "joana/testing/lexer_test_base.h"

namespace joana {
class AnalyzerSettings;
namespace analyzer {

class Context;

//
// AnalyzerTestBase
//
class AnalyzerTestBase : public LexerTestBase {
 protected:
  AnalyzerTestBase();
  ~AnalyzerTestBase() override;

  AnalyzerSettings& settings() { return *settings_; }

  std::unique_ptr<Context> NewContext();
  const ast::Node& ParseAsModule(base::StringPiece script_text);

 private:
  const std::unique_ptr<AnalyzerSettings> settings_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzerTestBase);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANALYZER_TEST_BASE_H_
