// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_temporary_change_value_h)
#define INCLUDE_common_temporary_change_value_h

#include "base/basictypes.h"

namespace common {

template<typename T>
class TemporaryChangeValue {
  private: T old_value_;
  private: T& value_;

  public: TemporaryChangeValue(T& value, const T& new_value)
      : old_value_(value), value_(value) {
    value_ = new_value;
  }

  public: ~TemporaryChangeValue() {
    value_ = old_value_;
  }

  DISALLOW_COPY_AND_ASSIGN(TemporaryChangeValue);
};

}  // namespace common

#endif //!defined(INCLUDE_common_temporary_change_value_h)
