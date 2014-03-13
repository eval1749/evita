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
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/range_set.h"
#include "evita/text/modes/mode.h"
#include "evita/text/undo_stack.h"

namespace text {

Buffer::Buffer(const base::string16& name, Mode* mode)
    : intervals_(new IntervalSet(this)),
      ranges_(new RangeSet(this)),
      m_pMode(mode),
      spelling_markers_(new MarkerSet(this)),
      style_resolver_(std::make_unique<css::StyleResolver>()),
      undo_stack_(new UndoStack(this)),
      m_eState(State_Ready),
      m_fReadOnly(false),
      m_nCharTick(1),
      m_nModfTick(1),
      m_nSaveTick(1),
      name_(name) {
  mode->set_buffer(this);
  spelling_markers_->AddObserver(this);
}

Buffer::~Buffer() {
  spelling_markers_->RemoveObserver(this);
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
  return GetIntervalAt(GetEnd())->GetStyle();
}

const css::Style& Buffer::GetStyleAt(Posn lPosn) const {
  return GetIntervalAt(lPosn)->GetStyle();
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

//  Markes the buffer visited to specified file.
void text::Buffer::SetFile(
    const base::string16& filename,
    base::Time last_write_time) {
  filename_ =  filename;
  last_write_time_ = last_write_time;
  m_nSaveTick = m_nCharTick;
}

void Buffer::SetMode(Mode* mode) {
  m_pMode = mode;
  m_pMode->set_buffer(this);
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
