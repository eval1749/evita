// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "joana/analyzer/public/analyzer_settings_builder.h"

#include "base/logging.h"

namespace joana {

//
// AnalyzerSettings::Builder
//
AnalyzerSettings::Builder::Builder() = default;
AnalyzerSettings::Builder::~Builder() = default;

AnalyzerSettings::Builder& AnalyzerSettings::Builder::set_error_sink(
    ErrorSink* error_sink) {
  DCHECK(error_sink);
  error_sink_ = error_sink;
  return *this;
}

AnalyzerSettings::Builder& AnalyzerSettings::Builder::set_zone(Zone* zone) {
  DCHECK(zone);
  zone_ = zone;
  return *this;
}

std::unique_ptr<AnalyzerSettings> AnalyzerSettings::Builder::Build() {
  return std::move(
      std::unique_ptr<AnalyzerSettings>(new AnalyzerSettings(*this)));
}

}  // namespace joana
