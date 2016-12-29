// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/context.h"

#include "joana/analyzer/factory.h"
#include "joana/analyzer/public/analyzer_settings.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"

namespace joana {
namespace analyzer {

//
// Context
//
Context::Context(const AnalyzerSettings& settings)
    : factory_(new Factory(&settings.zone())), settings_(settings) {}

Context::~Context() = default;

void Context::AddError(const SourceCodeRange& range, ErrorCode error_code) {
  settings_.error_sink().AddError(range, error_code);
}

}  // namespace analyzer
}  // namespace joana
