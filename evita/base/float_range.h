// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_FLOAT_RANGE_H_
#define EVITA_BASE_FLOAT_RANGE_H_

#include <iosfwd>

#include "evita/base/evita_base_export.h"

namespace base {

//////////////////////////////////////////////////////////////////////
//
// FloatRange
//
class EVITA_BASE_EXPORT FloatRange final {
 public:
  FloatRange(float lower, float upper);
  FloatRange(const FloatRange& other);
  FloatRange();
  ~FloatRange();

  FloatRange& operator=(const FloatRange& other);

  bool operator==(const FloatRange& other) const;
  bool operator!=(const FloatRange& other) const;

  bool empty() const { return lower_ == upper_; }
  float length() const { return upper_ - lower_; }
  float lower() const { return lower_; }
  float upper() const { return upper_; }

 private:
  float lower_ = 0;
  float upper_ = 0;
};

EVITA_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const FloatRange& range);

}  // namespace base

#endif  // EVITA_BASE_FLOAT_RANGE_H_
