// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ANALYZER_SETTINGS_BUILDER_H_
#define JOANA_ANALYZER_PUBLIC_ANALYZER_SETTINGS_BUILDER_H_

#include <memory>

#include "joana/analyzer/public/analyzer_settings.h"

namespace joana {

//
// AnalyzerSettings::Builder
//
class JOANA_ANALYZER_EXPORT AnalyzerSettings::Builder final {
 public:
  Builder();
  ~Builder();

  Builder& set_error_sink(ErrorSink* error_sink);
  Builder& set_zone(Zone* zone);

  std::unique_ptr<AnalyzerSettings> Build();

 private:
  friend class AnalyzerSettings;

  ErrorSink* error_sink_ = nullptr;
  Zone* zone_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ANALYZER_SETTINGS_BUILDER_H_
