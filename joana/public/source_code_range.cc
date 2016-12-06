// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/source_code_range.h"

#include "base/logging.h"
#include "joana/public/source_code.h"

namespace joana {

SourceCodeRange::SourceCodeRange(const SourceCode& source_code,
                                 int start,
                                 int end)
    : end_(end), source_code_(&source_code), start_(start) {
  DCHECK_GE(start_, 0);
  DCHECK_LE(start_, end_);
  DCHECK_LE(end_, source_code_->size());
  DCHECK(source_code_);
}

SourceCodeRange::SourceCodeRange(const SourceCodeRange& other) {
  source_code_ = other.source_code_;
  start_ = other.start_;
  end_ = other.end_;
}

bool SourceCodeRange::operator==(const SourceCodeRange& other) const {
  return source_code_ == other.source_code_ && start_ == other.start_ &&
         end_ == other.end_;
}

bool SourceCodeRange::operator!=(const SourceCodeRange& other) const {
  return !operator==(other);
}

base::StringPiece16 SourceCodeRange::GetString() const {
  return source_code_->GetString(start_, end_);
}

std::ostream& operator<<(std::ostream& ostream, const SourceCodeRange& range) {
  return ostream << '"' << range.source_code().file_path().value() << '('
                 << range.start() << ':' << range.end() << ")\"";
}

}  // namespace joana
