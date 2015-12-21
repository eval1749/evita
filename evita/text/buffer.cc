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
#include "evita/text/static_range.h"
#include "evita/text/undo_stack.h"

namespace text {

#if DCHECK_IS_ON()
#define DCHECK_NO_STATIC_RANGE() DCHECK(static_ranges_.empty());
#else
#define DCHECK_NO_STATIC_RANGE()
#endif

//////////////////////////////////////////////////////////////////////
//
// Buffer
//
Buffer::Buffer()
    : intervals_(new IntervalSet(this)),
      line_number_cache_(new LineNumberCache(*this)),
      ranges_(new RangeSet(this)),
      spelling_markers_(new MarkerSet(*this)),
      style_resolver_(new css::StyleResolver()),
      syntax_markers_(new MarkerSet(*this)),
      undo_stack_(new UndoStack(this)) {
  spelling_markers_->AddObserver(this);
  syntax_markers_->AddObserver(this);
}

Buffer::~Buffer() {
  spelling_markers_->RemoveObserver(this);
  syntax_markers_->RemoveObserver(this);
}

void Buffer::AddObserver(BufferMutationObserver* observer) const {
  const_cast<Buffer*>(this)->observers_.AddObserver(observer);
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

void Buffer::Delete(Offset start, Offset end) {
  DCHECK_NO_STATIC_RANGE();
  if (IsReadOnly())
    return;
  start = std::max(start, Offset());
  end = std::min(end, GetEnd());
  if (start == end)
    return;
  {
    const auto& range_for_will = StaticRange(*this, start, end);
    FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
                      WillDeleteAt(range_for_will));
  }
  deleteChars(start, end);
  UpdateChangeTick();
  const auto& range = StaticRange(*this, start, end);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_, DidDeleteAt(range));
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
  DCHECK_NO_STATIC_RANGE();
  if (text.empty())
    return;
  insert(offset, text.data(), text.size());

  UpdateChangeTick();
  const auto& range =
      StaticRange(*this, offset, offset + OffsetDelta(text.size()));
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_, DidInsertBefore(range));
}

Offset Buffer::Redo(Offset offset) {
  if (IsReadOnly())
    return Offset::Invalid();
  return undo_stack_->Redo(offset, 1);
}

void Buffer::RemoveObserver(BufferMutationObserver* observer) const {
  const_cast<Buffer*>(this)->observers_.RemoveObserver(observer);
}

void Buffer::ResetRevision(int revision) {
  DCHECK_NO_STATIC_RANGE();
  revision_ = revision;
}

void Buffer::SetStyle(Offset start, Offset end, const css::Style& style) {
  if (end > GetEnd())
    end = GetEnd();
  if (start == end)
    return;
  DCHECK_LT(start, end);
  intervals_->SetStyle(start, end, style);
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
  ++version_;
}

// IntervalSetObserver
void Buffer::DidChangeInterval(Offset start, Offset end) {
  DCHECK_LT(start, end);
  ++version_;
  const auto& range = StaticRange(*this, start, end);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_, DidChangeStyle(range));
}

// MarkerSetObserver
void Buffer::DidChangeMarker(Offset start, Offset end) {
  DCHECK_LT(start, end);
  ++version_;
  const auto& range = StaticRange(*this, start, end);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_, DidChangeStyle(range));
}

#if DCHECK_IS_ON()
// StaticRange
void Buffer::RegisterStaticRange(const StaticRange& range) {
  DCHECK(static_ranges_.find(&range) == static_ranges_.end());
  static_ranges_.insert(&range);
}

void Buffer::UnregisterStaticRange(const StaticRange& range) {
  const auto& it = static_ranges_.find(&range);
  DCHECK(it != static_ranges_.end());
  static_ranges_.erase(it);
}
#endif

}  // namespace text
