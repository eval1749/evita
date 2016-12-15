// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_LINE_NUMBER_CACHE_H_
#define JOANA_PUBLIC_LINE_NUMBER_CACHE_H_

#include <map>
#include <utility>

#include "base/macros.h"
#include "joana/public/public_export.h"

namespace joana {

class SourceCode;

class JOANA_PUBLIC_EXPORT LineNumberCache final {
 public:
  using LineNumber = int;
  using Offset = int;

  explicit LineNumberCache(const SourceCode& source_code);
  ~LineNumberCache();

  // Returns line number and start offset of line containing |offset|.
  std::pair<LineNumber, Offset> Get(int offset) const;

 private:
  bool IsEndOfLine(Offset offset) const;
  std::pair<LineNumber, Offset> UpdateCache(Offset offset) const;

  mutable std::map<LineNumber, Offset> map_;
  const SourceCode& source_code_;

  DISALLOW_COPY_AND_ASSIGN(LineNumberCache);
};

}  // namespace joana

#endif  // JOANA_PUBLIC_LINE_NUMBER_CACHE_H_
