// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/buffer.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/css/style_resolver.h"
#include "evita/text/interval.h"
#include "evita/text/interval_set.h"
#include "evita/text/line_number_cache.h"
#include "evita/text/marker_set.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "evita/text/range_set.h"
#include "evita/text/undo_stack.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Buffer
//
Buffer::Buffer()
    : intervals_(new IntervalSet(this)),
      line_number_cache_(new LineNumberCache(this)),
      ranges_(new RangeSet(this)),
      spelling_markers_(new MarkerSet(this)),
      style_resolver_(new css::StyleResolver()),
      syntax_markers_(new MarkerSet(this)),
      undo_stack_(new UndoStack(this)) {
  spelling_markers_->AddObserver(this);
  syntax_markers_->AddObserver(this);
}

Buffer::~Buffer() {
  spelling_markers_->RemoveObserver(this);
  syntax_markers_->RemoveObserver(this);
}

void Buffer::AddObserver(BufferMutationObserver* observer) {
  observers_.AddObserver(observer);
}

bool Buffer::CanRedo() const {
  return undo_stack_->CanRedo();
}

bool Buffer::CanUndo() const {
  return undo_stack_->CanUndo();
}

void Buffer::ClearUndo() {
  undo_stack_->Clear();
}

Offset Buffer::ComputeEndOfLine(Offset offset) const {
  DCHECK(IsValidPosn(offset));
  while (offset < GetEnd()) {
    if (GetCharAt(offset) == 0x0A)
      break;
    ++offset;
  }
  return offset;
}

Offset Buffer::ComputeStartOfLine(Offset offset) const {
  DCHECK(IsValidPosn(offset));
  while (offset > 0) {
    --offset;
    if (GetCharAt(offset) == 0x0A) {
      ++offset;
      break;
    }
  }
  return offset;
}

int Buffer::Delete(Offset start, Offset end) {
  if (IsReadOnly())
    return 0;

  start = std::max(start, Offset());
  end = std::min(end, GetEnd());

  auto const length = end - start;
  if (!length)
    return 0;

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                    WillDeleteAt(start, length));

  deleteChars(start, end);

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                    DidDeleteAt(start, length));

  UpdateChangeTick();
  return length;
}

void Buffer::EndUndoGroup(const base::string16& name) {
  undo_stack_->EndUndoGroup(name);
}

const css::Style& Buffer::GetDefaultStyle() const {
  return *css::Style::Default();
}

Interval* Buffer::GetIntervalAt(Offset offset) const {
  return intervals_->GetIntervalAt(std::min(offset, GetEnd()));
}

LineAndColumn Buffer::GetLineAndColumn(Offset offset) const {
  DCHECK(IsValidPosn(offset)) << "offset=" << offset << " length=" << GetEnd();
  auto const line_number_result = line_number_cache_->Get(offset);
  LineAndColumn result;
  result.line_number = line_number_result.number;
  result.column = offset - line_number_result.offset;
  return result;
}

const css::Style& Buffer::GetStyleAt(Offset offset) const {
  return GetIntervalAt(offset)->style();
}

void Buffer::InsertBefore(Offset offset, const base::string16& text) {
  DCHECK(IsValidPosn(offset));
  DCHECK(!IsReadOnly());

  auto const text_length = text.length();
  if (!text_length)
    return;

  insert(offset, text.data(), text_length);

  auto const delta = OffsetDelta(text_length);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                    DidInsertBefore(offset, delta));

  UpdateChangeTick();
}

Offset Buffer::Redo(Offset offset) {
  if (IsReadOnly())
    return Offset::Invalid();
  return undo_stack_->Redo(offset, 1);
}

void Buffer::RemoveObserver(BufferMutationObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Buffer::SetStyle(Offset start, Offset end, const css::Style& style) {
  if (end > GetEnd())
    end = GetEnd();
  if (start == end)
    return;
  DCHECK_LT(start, end);
  // To improve performance, we don't check contents of |style|.
  // This may be enough for syntax coloring.
  intervals_->SetStyle(start, end, style);
  auto const length = end - start;
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                    DidChangeStyle(start, length));
}

void Buffer::StartUndoGroup(const base::string16& name) {
  undo_stack_->BeginUndoGroup(name);
}

Offset Buffer::Undo(Offset offset) {
  if (IsReadOnly())
    return Offset::Invalid();
  return undo_stack_->Undo(offset, 1);
}

void Buffer::UpdateChangeTick() {
  ++revision_;
}

// MarkerSetObserver
void Buffer::DidChangeMarker(Offset start, Offset end) {
  DCHECK_LT(start, end);
  auto const length = end - start;
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                    DidChangeStyle(start, length));
}

}  // namespace text
