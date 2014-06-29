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
    : end_(end),
      start_(start),
      buffer_(buffer) {
  DCHECK(buffer_->IsValidRange(start_, end_));
  buffer_->ranges()->AddRange(this);
}

Range::~Range() {
  if (!buffer_)
    return;
  buffer_->ranges()->RemoveRange(this);
}

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

void Range::SetRange(Posn start, Posn end) {
  start = EnsureOffset(start);
  end = EnsureOffset(end);
  if (start <= end) {
    start_ = start;
    end_ = end;
  } else {
    start_ = end;
    end_ = start;
  }
}

void Range::set_start(Posn offset) {
  SetRange(offset, end_);
}

void Range::set_text(const base::string16& text) {
  if (buffer_->IsReadOnly()) {
    // TODO: We should throw read only buffer exception.
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
  start_ = start;
  end_ = EnsureOffset(static_cast<Posn>(start_ + text.length()));
}

Buffer::EnumChar::EnumChar(const Range* range)
    : m_lEnd(range->end()),
      m_lPosn(range->start()),
      m_pBuffer(range->buffer()) {
}

Buffer::EnumCharRev::EnumCharRev(const Range* range)
    : m_lStart(range->start()),
      m_lPosn(range->end()),
      m_pBuffer(range->buffer()) {
}

}  // namespace text
