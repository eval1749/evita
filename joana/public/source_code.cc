// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/source_code.h"

#include "joana/public/source_code_range.h"

namespace joana {

SourceCode::SourceCode(const base::FilePath& file_path,
                       base::StringPiece16 file_contents)
    : file_contents_(file_contents), file_path_(file_path) {}

SourceCode::~SourceCode() = default;

SourceCodeRange SourceCode::end() const {
  return SourceCodeRange(*this, size(), size());
}

SourceCodeRange SourceCode::range() const {
  return SourceCodeRange(*this, 0, size());
}

int SourceCode::size() const {
  return static_cast<int>(file_contents_.size());
}

SourceCodeRange SourceCode::start() const {
  return SourceCodeRange(*this, 0, 0);
}

base::char16 SourceCode::CharAt(int offset) const {
  DCHECK_GE(offset, 0);
  DCHECK_LT(static_cast<size_t>(offset), file_contents_.size());
  return file_contents_[offset];
}

base::StringPiece16 SourceCode::GetString(int start, int end) const {
  DCHECK_GE(start, 0);
  DCHECK_LE(start, end);
  DCHECK_LE(static_cast<size_t>(end), file_contents_.size());
  return file_contents_.substr(start, end - start);
}

SourceCodeRange SourceCode::Slice(int start, int end) const {
  DCHECK_GE(start, 0);
  DCHECK_LE(end, size());
  return SourceCodeRange(*this, start, end);
}

}  // namespace joana
