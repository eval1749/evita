// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_INTERVAL 0
#define DEBUG_STYLE 0
#include "evita/text/buffer.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/css/style_resolver.h"
#include "evita/text/interval.h"
#include "evita/text/interval_set.h"
#include "evita/text/line_number_cache.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/range_set.h"
#include "evita/text/undo_stack.h"

namespace text {

Buffer::Buffer(const base::string16& name)
    : intervals_(new IntervalSet(this)),
      line_number_cache_(new LineNumberCache(this)),
      ranges_(new RangeSet(this)),
      spelling_markers_(new MarkerSet(this)),
      style_resolver_(new css::StyleResolver()),
      syntax_markers_(new MarkerSet(this)),
      undo_stack_(new UndoStack(this)),
      m_eState(State_Ready),
      m_fReadOnly(false),
      m_nCharTick(1),
      m_nModfTick(1),
      m_nSaveTick(1),
      name_(name) {
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

/// <summary>
///   Returns true if this buffer undo-able.
/// </summary>
bool Buffer::CanUndo() const {
  return undo_stack_->CanUndo();
}

void Buffer::ClearUndo() {
  undo_stack_->Clear();
}

Posn Buffer::ComputeEndOfLine(Posn lPosn) const {
  DCHECK(IsValidPosn(lPosn));
  while (lPosn < GetEnd()) {
    if (0x0A == GetCharAt(lPosn))
      break;
    ++lPosn;
  }
  return lPosn;
}

Posn Buffer::ComputeStartOfLine(Posn lPosn) const {
  DCHECK(IsValidPosn(lPosn));
  while (lPosn > 0) {
    --lPosn;
    if (0x0A == GetCharAt(lPosn)) {
      ++lPosn;
      break;
    }
  }
  return lPosn;
}

Count Buffer::Delete(Posn lStart, Posn lEnd) {
  if (IsReadOnly())
    return 0;
  if (IsNotReady())
    return 0;

  lStart = std::max(lStart, static_cast<Posn>(0));
  lEnd   = std::min(lEnd, GetEnd());

  auto const length = lEnd - lStart;
  if (!length)
    return 0;

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      WillDeleteAt(lStart, static_cast<size_t>(length)));

  deleteChars(lStart, lEnd);
  ++m_nModfTick;

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidDeleteAt(lStart, static_cast<size_t>(length)));

  UpdateChangeTick();
  return length;
}

void Buffer::EndUndoGroup(const base::string16& name) {
  undo_stack_->EndUndoGroup(name);
}

const css::Style& Buffer::GetDefaultStyle() const {
  return GetIntervalAt(GetEnd())->style();
}

Interval* Buffer::GetIntervalAt(Posn lPosn) const {
  return intervals_->GetIntervalAt(std::min(lPosn, GetEnd()));
}

LineAndColumn Buffer::GetLineAndColumn(Posn offset) const {
  DCHECK(IsValidPosn(offset)) << "offset=" << offset << " length=" << GetEnd();
  auto const line_number_result = line_number_cache_->Get(offset);
  LineAndColumn result;
  result.line_number = line_number_result.number;
  result.column = offset - line_number_result.offset;
  return result;
}

const css::Style& Buffer::GetStyleAt(Posn lPosn) const {
  return GetIntervalAt(lPosn)->style();
}

Count Buffer::Insert(Posn lPosn, const char16* pwch, Count n) {
  DCHECK(IsValidPosn(lPosn));

  if (IsReadOnly())
    return 0;
  if (IsNotReady())
    return 0;

  if (n <= 0)
    return 0;
  lPosn = std::min(lPosn, GetEnd());
  insert(lPosn, pwch, n);
  ++m_nModfTick;
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidInsertAt(lPosn, static_cast<size_t>(n)));
  UpdateChangeTick();

  return n;
}

void Buffer::InsertBefore(Posn position, const base::string16& text) {
  DCHECK(IsValidPosn(position));
  DCHECK(!IsReadOnly());
  DCHECK_NE(State_Save, m_eState);

  auto const text_length = text.length();
  if (!text_length)
    return;

  insert(position, text.data(), static_cast<Count>(text_length));
  ++m_nModfTick;

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidInsertBefore(position, text_length));

  UpdateChangeTick();
}

bool Buffer::IsNotReady() const {
  return m_eState != State_Ready;
}

Posn Buffer::Redo(Posn lPosn, Count n) {
  if (IsReadOnly())
    return -1;
  return undo_stack_->Redo(lPosn, n);
}

void Buffer::RemoveObserver(BufferMutationObserver* observer) {
  observers_.RemoveObserver(observer);
}

void text::Buffer::SetModified(bool new_modified) {
  if (IsModified() == new_modified)
    return;
  if (new_modified)
    UpdateChangeTick();
  else
    m_nSaveTick = m_nCharTick;
}

void Buffer::SetStyle(Posn lStart, Posn lEnd, const css::Style& style) {
  if (lStart < 0)
    lStart = 0;
  if (lEnd > GetEnd())
    lEnd = GetEnd();
  if (lStart == lEnd)
    return;
  DCHECK_LT(lStart, lEnd);
  // To improve performance, we don't check contents of |style|.
  // This may be enough for syntax coloring.
  m_nModfTick += 1;
  intervals_->SetStyle(lStart, lEnd, style);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
    DidChangeStyle(lStart, static_cast<size_t>(lEnd - lStart)));
}

void Buffer::StartUndoGroup(const base::string16& name) {
  undo_stack_->BeginUndoGroup(name);
}

Posn Buffer::Undo(Posn lPosn, Count n) {
  if (IsReadOnly())
    return -1;
  return undo_stack_->Undo(lPosn, n);
}

void Buffer::UpdateChangeTick() {
  if (m_nCharTick < m_nSaveTick)
    m_nCharTick = m_nSaveTick;
  ++m_nCharTick;
}

// MarkerSetObserver
void Buffer::DidChangeMarker(Posn start, Posn end) {
  DCHECK_LT(start, end);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
    DidChangeStyle(start, static_cast<size_t>(end - start)));
}

//////////////////////////////////////////////////////////////////////
//
// UndoBlock
//
UndoBlock::~UndoBlock() {
  buffer_->EndUndoGroup(name_);
}

UndoBlock::UndoBlock(Buffer* buffer, const base::string16& name)
    : buffer_(buffer), name_(name) {
  buffer->StartUndoGroup(name_);
}

}  // namespace text
