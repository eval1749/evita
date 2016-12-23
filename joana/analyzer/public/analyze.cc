// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyze.h"

#include "joana/analyzer/analyzer.h"

namespace joana {

using Analyzer = internal::Analyzer;

//
// AnalyzeContext
//
AnalyzeContext::AnalyzeContext(Zone* zone,
                               ErrorSink* error_sink,
                               const Options& options)
    : error_sink_(*error_sink), options_(options), zone_(*zone) {}

AnalyzeContext::~AnalyzeContext() = default;

// Options
AnalyzeContext::Options::Options(const Options& other) = default;
AnalyzeContext::Options::Options() = default;
AnalyzeContext::Options::~Options() = default;

// Options::Builder
AnalyzeContext::Options::Builder::Builder() = default;
AnalyzeContext::Options::Builder::~Builder() = default;

const AnalyzeContext::Options AnalyzeContext::Options::Builder::Build() {
  return options_;
}

//
// Parse; the entry point
//
const ast::Node& Analyze(AnalyzeContext* context, const ast::Node& node) {
  Analyzer analyzer(context, node);
  return analyzer.Run();
}

}  // namespace joana
