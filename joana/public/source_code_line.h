// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_SOURCE_CODE_LINE_H_
#define JOANA_PUBLIC_SOURCE_CODE_LINE_H_

#include "joana/public/source_code_range.h"

namespace joana {

// Line number starts with one.
class JOANA_PUBLIC_EXPORT SourceCodeLine final {
 public:
  class Cache;

  SourceCodeLine(const SourceCodeLine& other);
  ~SourceCodeLine();

  int number() const { return number_; }
  const SourceCodeRange& range() const { return range_; }

  bool operator==(const SourceCodeLine& other) const;
  bool operator!=(const SourceCodeLine& other) const;

 private:
  friend class SourceCodeLineTest;

  SourceCodeLine(const SourceCodeRange& range, int number);

  SourceCodeRange range_;
  int number_ = 0;
};

}  // namespace joana

#endif  // JOANA_PUBLIC_SOURCE_CODE_LINE_H_
