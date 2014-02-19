// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_INTERVAL 0
#define DEBUG_STYLE 0
#include "evita/text/buffer.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/ed_Interval.h"
#include "evita/text/interval_set.h"
#include "evita/text/range.h"
#include "evita/text/range_list.h"
#include "evita/ed_Undo.h"
#include "evita/text/modes/mode.h"

namespace text {

Buffer::Buffer(const base::string16& name, Mode* mode)
    : m_hObjHeap(::HeapCreate(HEAP_NO_SERIALIZE, 0, 0)),
      intervals_(new IntervalSet(this)),
      ranges_(new RangeList(this)),
      m_pMode(mode),
      m_pUndo(new(m_hObjHeap) UndoManager(this)),
      m_eState(State_Ready),
      m_fReadOnly(false),
      m_fUndo(true),
      m_nCharTick(1),
      m_nModfTick(1),
      m_nSaveTick(1),
      name_(name) {
  mode->set_buffer(this);
}

Buffer::~Buffer() {
  if (m_hObjHeap)
    ::HeapDestroy(m_hObjHeap);
}

void Buffer::AddObserver(BufferMutationObserver* observer) {
  observers_.AddObserver(observer);
}

bool Buffer::CanRedo() const {
  if (!m_fUndo || !m_pUndo)
    return false;
  return m_pUndo->CanRedo();
}

/// <summary>
///   Returns true if this buffer undo-able.
/// </summary>
bool Buffer::CanUndo() const
{
  if (!m_fUndo || !m_pUndo)
    return false;
  return m_pUndo->CanUndo();
}

void Buffer::ClearUndo() {
  if (!m_pUndo)
    return;
  m_pUndo->Empty();
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

  if (lEnd <= lStart)
    return 0;

  if (m_fUndo && m_pUndo) {
    m_pUndo->CheckPoint();
    m_pUndo->RecordDelete(lStart, lEnd);
  }

  InternalDelete(lStart, lEnd);
  onChange();
  return lEnd - lStart;
}

const StyleValues* Buffer::GetDefaultStyle() const {
  return GetIntervalAt(GetEnd())->GetStyle();
}

const StyleValues* Buffer::GetStyleAt(Posn lPosn) const {
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

  InternalInsert(lPosn, pwch, n);
  onChange();

  if (m_fUndo && m_pUndo) {
    m_pUndo->CheckPoint();
    m_pUndo->RecordInsert(lPosn, lPosn + n);
  }

  return n;
}

void Buffer::InsertBefore(Posn position, const base::string16& text) {
  DCHECK(IsValidPosn(position));
  DCHECK(!IsReadOnly());
  DCHECK_NE(State_Save, m_eState);

  auto const text_length = text.length();
  insert(position, text.data(), static_cast<Count>(text_length));
  ++m_nModfTick;

  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidInsertBefore(position, text_length));

  auto const change_end = static_cast<Posn>(position + text_length);

  // Inserted text inherites style before insertion point.
  SetStyle(position, change_end, position ? GetStyleAt(position - 1) :
                                            GetDefaultStyle());

  foreach (ChangeTrackers::Enum, enum_tracker, &m_oChangeTrackers) {
    auto const tracker = enum_tracker.Get();
    tracker->m_lStart = std::min(tracker->m_lStart, position);
    tracker->m_lEnd = std::max(tracker->m_lEnd, change_end);
  }

  onChange();
  if (m_fUndo && m_pUndo) {
    m_pUndo->CheckPoint();
    m_pUndo->RecordInsert(position, static_cast<Posn>(position + text_length));
  }
}

void Buffer::InternalDelete(Posn lStart, Posn lEnd) {
  Count n = deleteChars(lStart, lEnd);
  ++m_nModfTick;
  relocate(lStart, -n);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidDeleteAt(lStart, static_cast<size_t>(n)));
}

void Buffer::InternalInsert(Posn lPosn, const char16* pwch, Count n) {
  insert(lPosn, pwch, n);
  ++m_nModfTick;
  relocate(lPosn, n);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
      DidDeleteAt(lPosn, static_cast<size_t>(n)));
}

bool Buffer::IsNotReady() const {
  return m_eState != State_Ready;
}

void Buffer::onChange() {
  if (m_nCharTick < m_nSaveTick)
    m_nCharTick = m_nSaveTick;
  ++m_nCharTick;
}

Posn Buffer::Redo(Posn lPosn, Count n) {
  if (IsReadOnly() || !m_fUndo || !m_pUndo)
    return -1;
  return m_pUndo->Redo(lPosn, n);
}

void Buffer::relocate(Posn lPosn, Count iDelta) {
  // Update change trackers
  {
    Posn lChangeStart = lPosn;
    Posn lChangeEnd   = lPosn;

    if (iDelta > 0)
      lChangeEnd += iDelta;

    foreach (ChangeTrackers::Enum, oEnum, &m_oChangeTrackers) {
      auto const pRunner = oEnum.Get();
      pRunner->m_lStart = std::min(pRunner->m_lStart, lChangeStart);
      pRunner->m_lEnd   = std::max(pRunner->m_lEnd, lChangeEnd),
      pRunner->m_lEnd = std::min(pRunner->m_lEnd, GetEnd());
    }
  }
}

void Buffer::SetMode(Mode* mode) {
  m_pMode = mode;
  m_pMode->set_buffer(this);
}

Posn Buffer::Undo(Posn lPosn, Count n) {
  if (IsReadOnly() || !m_fUndo || !m_pUndo)
    return -1;
  return m_pUndo->Undo(lPosn, n);
}

}  // namespace text
