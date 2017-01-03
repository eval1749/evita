// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/analyzer_test_base.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/public/analyzer_settings_builder.h"
#include "joana/parser/public/parse.h"
#include "joana/testing/simple_error_sink.h"

namespace joana {
namespace analyzer {

//
// AnalyzerTestBase
//
AnalyzerTestBase::AnalyzerTestBase()
    : settings_(AnalyzerSettings::Builder()
                    .set_error_sink(&error_sink())
                    .set_zone(&zone())
                    .Build()),
      context_(std::make_unique<Context>(*settings_)) {}

AnalyzerTestBase::~AnalyzerTestBase() = default;

const ast::Node& AnalyzerTestBase::ParseAsModule(
    base::StringPiece script_text) {
  PrepareSouceCode(script_text);
  ParserOptions options;
  return Parse(&context(), source_code().range(), options);
}

}  // namespace analyzer
}  // namespace joana
