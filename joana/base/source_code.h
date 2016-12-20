// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_SOURCE_CODE_H_
#define JOANA_BASE_SOURCE_CODE_H_

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "joana/base/base_export.h"

namespace joana {

class SourceCodeRange;

class JOANA_BASE_EXPORT SourceCode final {
 public:
  class Factory;

  ~SourceCode();

  SourceCodeRange end() const;
  const base::FilePath& file_path() const { return file_path_; }
  const base::StringPiece16 contents() const { return file_contents_; }
  SourceCodeRange range() const;
  int size() const;
  SourceCodeRange start() const;

  base::char16 CharAt(int offset) const;
  base::StringPiece16 GetString(int start, int end) const;
  SourceCodeRange Slice(int start, int end) const;

 private:
  SourceCode(const base::FilePath& file_path,
             base::StringPiece16 file_contents);

  const base::StringPiece16 file_contents_;
  const base::FilePath file_path_;

  DISALLOW_COPY_AND_ASSIGN(SourceCode);
};

}  // namespace joana

#endif  // JOANA_BASE_SOURCE_CODE_H_
