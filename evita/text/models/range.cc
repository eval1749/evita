// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/models/range.h"

#include <algorithm>
#include <utility>

#include "base/logging.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/range_set.h"
#include "evita/text/models/scoped_undo_group.h"

namespace text {

Range::Range(Buffer* buffer, Offset start, Offset end)
    : end_(end), start_(start), buffer_(buffer) {
  DCHECK(buffer_->IsValidRange(start_, end_));
  buffer_->ranges()->AddRange(this);
}

Range::~Range() {
  if (!buffer_)
    return;
  buffer_->ranges()->RemoveRange(this);
}

void Range::DidChangeRange() {}

Offset Range::EnsureOffset(Offset offset) const {
  if (!offset.IsValid())
    return Offset();
  if (offset > buffer_->GetEnd())
    return buffer_->GetEnd();
  return offset;
}

base::string16 Range::text() const {
  return buffer_->GetText(start_, end_);
}

void Range::set_end(Offset offset) {
  SetRange(start_, offset);
}

void Range::SetRange(Offset new_start, Offset new_end) {
  new_start = EnsureOffset(new_start);
  new_end = EnsureOffset(new_end);
  if (new_start > new_end)
    std::swap(new_start, new_end);
  if (start_ == new_start && end_ == new_end)
    return;
  start_ = new_start;
  end_ = new_end;
  DidChangeRange();
}

void Range::set_start(Offset offset) {
  SetRange(offset, end_);
}

void Range::set_text(const base::string16& text) {
  DCHECK(!buffer_->IsReadOnly());
  const auto start = start_;
  {
    ScopedUndoGroup undo_scope(this, L"Range.SetText");
    buffer_->Replace(start_, end_, text);
  }
  const auto end = start + OffsetDelta(text.length());
  SetRange(start, EnsureOffset(end));
}

}  // namespace text
