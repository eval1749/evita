// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/range_set.h"
#include "evita/text/undo_stack.h"

namespace text {

Range::Range(Buffer* pBuffer, Posn lStart, Posn lEnd)
    : end_(lEnd),
      start_(lStart),
      buffer_(pBuffer) {
  DCHECK(buffer_->IsValidRange(start_, end_));
  buffer_->ranges()->AddRange(this);
}

Range::~Range() {
  if (buffer_)
    buffer_->ranges()->RemoveRange(this);
}

Posn Range::ensurePosn(Posn lPosn) const {
  if (lPosn < 0)
    return 0;
  if (lPosn > buffer_->GetEnd())
    return buffer_->GetEnd();
  return lPosn;
}

// FIXME 2007-07-18 yosi We should stop if counter reaches zero.
void Range::GetInformation(Information* out_oInfo, Count n) const {
  Count k = n;
  out_oInfo->m_lLineNum = 1;
  for (Posn lPosn = 0; lPosn < start_; lPosn++) {
    if (buffer_->GetCharAt(lPosn) == 0x0A)
      ++out_oInfo->m_lLineNum;
  }

  out_oInfo->m_fLineNum = k > 0;
  auto const lLineStart = buffer_->ComputeStartOfLine(start_);
  out_oInfo->m_fColumn = (start_ - lLineStart) < n;
  out_oInfo->m_lColumn = start_ - lLineStart;
}

base::string16 Range::GetText() const {
  return buffer_->GetText(start_, end_);
}

Posn Range::SetEnd(Posn lPosn) {
  SetRange(start_, lPosn);
  return end_;
}

void Range::SetRange(Posn lStart, Posn lEnd) {
  lStart = ensurePosn(lStart);
  lEnd = ensurePosn(lEnd);
  if (lStart > lEnd)
    swap(lStart, lEnd);
  start_ = lStart;
  end_ = lEnd;
}

Posn Range::SetStart(Posn lPosn) {
  SetRange(lPosn, end_);
  return start_;
}

void Range::SetText(const base::string16& text) {
  if (buffer_->IsReadOnly()) {
    // TODO: We should throw read only buffer exception.
    return;
  }

  if (start_ == end_) {
    UndoBlock oUndo(this, L"Range.SetText");
    buffer_->Insert(start_, text.data(), static_cast<Count>(text.length()));
  } else {
    UndoBlock oUndo(this, L"Range.SetText");
    buffer_->Delete(start_, end_);
    buffer_->Insert(start_, text.data(), static_cast<Count>(text.length()));
  }

  end_ = ensurePosn(static_cast<Posn>(start_ + text.length()));
}

Buffer::EnumChar::EnumChar(const Range* pRange)
    : m_lEnd(pRange->GetEnd()),
      m_lPosn(pRange->GetStart()),
      m_pBuffer(pRange->buffer()) {
}

Buffer::EnumCharRev::EnumCharRev(const Range* pRange)
    : m_lStart(pRange->GetStart()),
      m_lPosn(pRange->GetEnd()),
      m_pBuffer(pRange->buffer()) {
}

UndoBlock::UndoBlock(Range* range, const base::string16& name)
    : UndoBlock(range->buffer(), name) {
}

}  // namespace text
