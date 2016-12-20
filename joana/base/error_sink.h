// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_ERROR_SINK_H_
#define JOANA_BASE_ERROR_SINK_H_

#include "base/macros.h"
#include "joana/base/base_export.h"

namespace joana {

class SourceCodeRange;

class JOANA_BASE_EXPORT ErrorSink {
 public:
  virtual void AddError(const SourceCodeRange& range, int error_code) = 0;

 protected:
  ErrorSink();
  ~ErrorSink();

 private:
  DISALLOW_COPY_AND_ASSIGN(ErrorSink);
};

}  // namespace joana

#endif  // JOANA_BASE_ERROR_SINK_H_
