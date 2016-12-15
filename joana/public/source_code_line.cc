// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/source_code_line.h"

#include "base/logging.h"

namespace joana {

SourceCodeLine::SourceCodeLine(const SourceCodeRange& range, int number)
    : number_(number), range_(range) {
  DCHECK_GE(number_, 1);
}

SourceCodeLine::SourceCodeLine(const SourceCodeLine& other)
    : number_(other.number_), range_(other.range_) {}

SourceCodeLine::~SourceCodeLine() = default;

}  // namespace joana
