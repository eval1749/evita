// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <type_traits>

#ifndef JOANA_BASE_ERROR_SINK_H_
#define JOANA_BASE_ERROR_SINK_H_

#include "base/macros.h"
#include "joana/base/base_export.h"

namespace joana {

class SourceCodeRange;

class JOANA_BASE_EXPORT ErrorSink {
 public:
  virtual void AddError(const SourceCodeRange& range, int error_code) = 0;

  template <typename T>
  void AddError(const SourceCodeRange& range, T error_code) {
    static_assert(std::is_enum<T>::value, "T is must be an enum type.");
    AddError(range, static_cast<int>(error_code));
  }

 protected:
  ErrorSink();
  ~ErrorSink();

 private:
  DISALLOW_COPY_AND_ASSIGN(ErrorSink);
};

}  // namespace joana

#endif  // JOANA_BASE_ERROR_SINK_H_
