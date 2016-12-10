// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/simple_error_sink.h"

namespace joana {
namespace internal {

SimpleErrorSink::Error::Error(const SourceCodeRange& range, int error_code)
    : error_code_(error_code), range_(range) {}

SimpleErrorSink::Error::~Error() = default;

void SimpleErrorSink::AddError(const SourceCodeRange& range, int error_code) {
  auto* const record = new (&zone_) Error(range, error_code);
  errors_.push_back(record);
}

SimpleErrorSink::SimpleErrorSink() : zone_("SimpleErrorSink") {}
SimpleErrorSink::~SimpleErrorSink() = default;

void SimpleErrorSink::Reset() {
  errors_.clear();
}

}  // namespace internal
}  // namespace joana
