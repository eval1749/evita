// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_LINE_NUMBER_CACHE_H_
#define JOANA_BASE_LINE_NUMBER_CACHE_H_

#include <utility>
#include <vector>

#include "base/macros.h"
#include "joana/base/base_export.h"

namespace joana {

class SourceCode;

class JOANA_BASE_EXPORT LineNumberCache final {
 public:
  // Zero origin column number
  using ColumnNumber = int;

  // One origin line number.
  using LineNumber = int;

  // Zero origin offset in source code. The unit is UTF-16 code point.
  using Offset = int;

  explicit LineNumberCache(const SourceCode& source_code);
  ~LineNumberCache();

  // Returns line number and column number in the line containing |offset|.
  std::pair<LineNumber, ColumnNumber> Get(int offset) const;

 private:
  bool IsEndOfLine(Offset offset) const;

  // Last scanned offset.
  mutable Offset last_offset_ = 0;

  // First element is start offset of second line. The first line start at
  // offset zero.
  mutable std::vector<Offset> line_start_offsets_;

  // The source code provider.
  const SourceCode& source_code_;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCache);
};

}  // namespace joana

#endif  // JOANA_BASE_LINE_NUMBER_CACHE_H_
