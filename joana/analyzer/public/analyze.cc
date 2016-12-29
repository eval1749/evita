// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/public/analyze.h"

#include "joana/analyzer/analyzer.h"

namespace joana {

using Analyzer = analyzer::Analyzer;

//
// AnalyzerSettings
//
AnalyzerSettings::AnalyzerSettings(Zone* zone,
                                   ErrorSink* error_sink,
                                   const Options& options)
    : error_sink_(*error_sink), options_(options), zone_(*zone) {}

AnalyzerSettings::~AnalyzerSettings() = default;

// Options
AnalyzerSettings::Options::Options(const Options& other) = default;
AnalyzerSettings::Options::Options() = default;
AnalyzerSettings::Options::~Options() = default;

// Options::Builder
AnalyzerSettings::Options::Builder::Builder() = default;
AnalyzerSettings::Options::Builder::~Builder() = default;

const AnalyzerSettings::Options AnalyzerSettings::Options::Builder::Build() {
  return options_;
}

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
