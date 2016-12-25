// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_TESTING_SIMPLE_ERROR_SINK_H_
#define JOANA_TESTING_SIMPLE_ERROR_SINK_H_

#include <iosfwd>
#include <vector>

#include "base/macros.h"
#include "joana/base/error_sink.h"
#include "joana/base/memory/zone.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/source_code_range.h"

namespace joana {

//
// SimpleErrorSink
//
class SimpleErrorSink final : public ErrorSink {
 public:
  class Error final : public ZoneAllocated {
   public:
    Error(const SourceCodeRange& range, int error_code);
    ~Error();

    int error_code() const { return error_code_; }
    const SourceCodeRange& range() const { return range_; }

   private:
    const int error_code_;
    const SourceCodeRange range_;

    DISALLOW_COPY_AND_ASSIGN(Error);
  };

  SimpleErrorSink();
  ~SimpleErrorSink();

  const std::vector<Error*>& errors() const { return errors_; }

  void Reset();

 private:
  // |ErrorSink| members
  void AddError(const SourceCodeRange& range, int error_code) final;

  Zone zone_;
  std::vector<Error*> errors_;

  DISALLOW_COPY_AND_ASSIGN(SimpleErrorSink);
};

std::ostream& operator<<(std::ostream& ostream,
                         const SimpleErrorSink::Error& error);
std::ostream& operator<<(std::ostream& ostream,
                         const SimpleErrorSink::Error* error);

}  // namespace joana

#endif  // JOANA_TESTING_SIMPLE_ERROR_SINK_H_
