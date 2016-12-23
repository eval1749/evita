// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_SOURCE_CODE_RANGE_H_
#define JOANA_BASE_SOURCE_CODE_RANGE_H_

#include <iosfwd>

#include "base/strings/string_piece.h"
#include "joana/base/base_export.h"

namespace joana {

class SourceCode;

class JOANA_BASE_EXPORT SourceCodeRange final {
 public:
  SourceCodeRange(const SourceCodeRange& other);
  SourceCodeRange() = delete;

  bool operator==(const SourceCodeRange& other) const;
  bool operator!=(const SourceCodeRange& other) const;

  int end() const { return end_; }
  const SourceCode& source_code() const { return *source_code_; }
  int size() const { return end_ - start_; }
  int start() const { return start_; }

  bool Contains(int offset) const;
  base::StringPiece16 GetString() const;
  bool IsCollapsed() const;

  static SourceCodeRange CollapseToEnd(const SourceCodeRange& range);
  static SourceCodeRange CollapseToStart(const SourceCodeRange& range);

  static SourceCodeRange Merge(const SourceCodeRange& range1,
                               const SourceCodeRange& range2);

  template <typename... Params>
  static SourceCodeRange Merge(const SourceCodeRange& range1,
                               const SourceCodeRange& range2,
                               Params... params);

 private:
  friend class SourceCode;

  SourceCodeRange(const SourceCode& source_code, int start, int end);

  int end_;
  const SourceCode* source_code_;
  int start_;
};

JOANA_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                             const SourceCodeRange& range);

template <typename... Params>
SourceCodeRange SourceCodeRange::Merge(const SourceCodeRange& range1,
                                       const SourceCodeRange& range2,
                                       Params... params) {
  return Merge(range1, Merge(range2, params...));
}

}  // namespace joana

#endif  // JOANA_BASE_SOURCE_CODE_RANGE_H_