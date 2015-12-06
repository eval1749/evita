// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/range_set.h"
#include "evita/text/scoped_undo_group.h"

namespace text {

Range::Range(Buffer* buffer, Posn start, Posn end)
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

Posn Range::EnsureOffset(Posn offset) const {
  if (offset < 0)
    return 0;
  if (offset > buffer_->GetEnd())
    return buffer_->GetEnd();
  return offset;
}

base::string16 Range::text() const {
  return buffer_->GetText(start_, end_);
}

void Range::set_end(Posn offset) {
  SetRange(start_, offset);
}

void Range::SetRange(Posn new_start, Posn new_end) {
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

void Range::set_start(Posn offset) {
  SetRange(offset, end_);
}

void Range::set_text(const base::string16& text) {
  if (buffer_->IsReadOnly()) {
    // TODO(eval1749): We should throw read only buffer exception.
    return;
  }

  auto const start = start_;
  if (start_ == end_) {
    ScopedUndoGroup oUndo(this, L"Range.SetText");
    buffer_->InsertBefore(start_, text);
  } else {
    ScopedUndoGroup oUndo(this, L"Range.SetText");
    buffer_->Delete(start_, end_);
    buffer_->InsertBefore(start_, text);
  }
  SetRange(start, EnsureOffset(static_cast<Posn>(start + text.length())));
}

}  // namespace text
