// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/testing/simple_error_sink.h"

#include "joana/ast/error_codes.h"

namespace joana {

//
// SimpleErrorSink
//
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

std::ostream& operator<<(std::ostream& ostream,
                         const SimpleErrorSink::Error& error) {
  ostream << ast::ErrorStringOf(error.error_code()) << '@'
          << error.range().start() << ':' << error.range().end();
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream,
                         const SimpleErrorSink::Error* error) {
  if (!error)
    return ostream << "(null)";
  return ostream << *error;
}

}  // namespace joana
