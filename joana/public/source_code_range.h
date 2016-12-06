// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_SOURCE_CODE_RANGE_H_
#define JOANA_PUBLIC_SOURCE_CODE_RANGE_H_

#include <iosfwd>

#include "base/strings/string_piece.h"
#include "joana/public/public_export.h"

namespace joana {

class SourceCode;

class JOANA_PUBLIC_EXPORT SourceCodeRange final {
 public:
  SourceCodeRange(const SourceCodeRange& other);
  SourceCodeRange() = delete;

  bool operator==(const SourceCodeRange& other) const;
  bool operator!=(const SourceCodeRange& other) const;

  int end() const { return end_; }
  const SourceCode& source_code() const { return *source_code_; }
  int start() const { return start_; }

  base::StringPiece16 GetString() const;

 private:
  friend class SourceCode;

  SourceCodeRange(const SourceCode& source_code, int start, int end);

  int end_;
  const SourceCode* source_code_;
  int start_;
};

JOANA_PUBLIC_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                             const SourceCodeRange& range);

}  // namespace joana

#endif  // JOANA_PUBLIC_SOURCE_CODE_RANGE_H_
