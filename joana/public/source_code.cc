// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/source_code.h"

namespace joana {

SourceCode::SourceCode(const base::FilePath& file_path,
                       base::StringPiece16 file_contents)
    : file_contents_(file_contents), file_path_(file_path) {}

SourceCode::~SourceCode() = default;

base::StringPiece16 SourceCode::GetString(int start, int end) const {
  DCHECK_GE(start, 0);
  DCHECK_LE(start, end);
  DCHECK_LE(static_cast<size_t>(end), file_contents_.size());
  return file_contents_.substr(start, end - start);
}

}  // namespace joana
