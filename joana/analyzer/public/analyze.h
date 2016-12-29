// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ANALYZE_H_
#define JOANA_ANALYZER_PUBLIC_ANALYZE_H_

#include <memory>

#include "base/macros.h"
#include "joana/analyzer/public/analyzer_export.h"

namespace joana {
class ErrorSink;
class Zone;

namespace ast {
class Node;
}

class SourceCodeRange;

//
// AnalyzerSettings
//
class JOANA_ANALYZER_EXPORT AnalyzerSettings final {
 public:
  // Options
  class JOANA_ANALYZER_EXPORT Options final {
   public:
    class Builder;

    Options(const Options& others);
    ~Options();

   private:
    Options();

    bool dummy_ = false;
  };

  AnalyzerSettings(Zone* zone, ErrorSink* error_sink, const Options& options);
  ~AnalyzerSettings();

  ErrorSink& error_sink() const { return error_sink_; }
  const Options& options() const { return options_; }
  Zone& zone() const { return zone_; }

 private:
  ErrorSink& error_sink_;
  const Options options_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(AnalyzerSettings);
};

//
// AnalyzerSettings::Options::Builder
//
class JOANA_ANALYZER_EXPORT AnalyzerSettings::Options::Builder final {
 public:
  Builder();
  ~Builder();

  const Options Build();

 private:
  Options options_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

//
// The analyzer entry point.
//
JOANA_ANALYZER_EXPORT const ast::Node& AddExterns(AnalyzerSettings* context,
                                                  const ast::Node& node);

JOANA_ANALYZER_EXPORT const ast::Node& Analyze(AnalyzerSettings* context,
                                               const ast::Node& node);

}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ANALYZE_H_
