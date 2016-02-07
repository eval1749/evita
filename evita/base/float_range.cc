// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/base/float_range.h"

#include "base/logging.h"

namespace base {

//////////////////////////////////////////////////////////////////////
//
// FloatRange
//
FloatRange::FloatRange(float lower, float upper)
    : lower_(lower), upper_(upper) {
  DCHECK_LE(lower_, upper_);
}

FloatRange::FloatRange(const FloatRange& other)
    : FloatRange(other.lower_, other.upper_) {}

FloatRange::FloatRange() : FloatRange(0, 0) {}
FloatRange::~FloatRange() {}

FloatRange& FloatRange::operator=(const FloatRange& other) {
  lower_ = other.lower_;
  upper_ = other.upper_;
  return *this;
}

bool FloatRange::operator==(const FloatRange& other) const {
  return lower_ == other.lower_ && upper_ == other.upper_;
}

bool FloatRange::operator!=(const FloatRange& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const FloatRange& range) {
  return ostream << "FloatRange(" << range.lower() << ',' << range.upper()
                 << ')';
}

}  // namespace base
