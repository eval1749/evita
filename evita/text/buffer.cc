// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_INTERVAL 0
#define DEBUG_STYLE 0
#include "evita/text/buffer.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/ed_Interval.h"
#include "evita/text/range.h"
#include "evita/ed_Undo.h"
#include "evita/text/modes/mode.h"

namespace text {

Buffer::Buffer(const base::string16& name, Mode* mode)
    : m_hObjHeap(::HeapCreate(HEAP_NO_SERIALIZE, 0, 0)),
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

  {
    auto const interval = newInterval(0, 1);
    interval->SetStyle(&g_DefaultStyle);
    m_oIntervalTree.Insert(interval);
    m_oIntervals.Append(interval);
  }
}

Buffer::~Buffer() {
  for (auto* range : ranges_) {
    range->m_pBuffer = nullptr;
  }

  if (m_hObjHeap)
    ::HeapDestroy(m_hObjHeap);
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
  return m_oIntervals.GetLast()->GetStyle();
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

  for (auto* range : ranges_) {
    if (range->m_lStart >= position)
      range->m_lStart += text_length;
    if (range->m_lEnd >= position)
      range->m_lEnd += text_length;
  }

  foreach (EnumInterval, enum_interval, this) {
    auto const interval = enum_interval.Get();
    if (interval->m_lStart >= position)
      interval->m_lStart += text_length;
    if (interval->m_lEnd >= position)
      interval->m_lEnd += text_length;
  }

  if (!position) {
    // Set default style to new text inserted at start of document.
    auto const head = m_oIntervals.GetFirst();
    DCHECK_EQ(static_cast<Posn>(text_length), head->GetStart());
    // TODO(yosi) We should check head interval has default style or not
    // without creating Interval object.
    auto const interval = newInterval(0, head->GetStart());
    if (interval->CanMerge(head)) {
      head->m_lStart = 0;
      destroyObject(interval);
    } else {
      m_oIntervals.Prepend(interval);
      m_oIntervalTree.Insert(interval);
    }
  }

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

Range* Buffer::InternalAddRange(Range* pRange) {
  ranges_.insert(pRange);
  return pRange;
}

void Buffer::InternalDelete(Posn lStart, Posn lEnd) {
  Count n = deleteChars(lStart, lEnd);
  ++m_nModfTick;
  relocate(lStart, -n);
}

void Buffer::InternalInsert(Posn lPosn, const char16* pwch, Count n) {
  insert(lPosn, pwch, n);
  ++m_nModfTick;
  relocate(lPosn, n);
}

void Buffer::InternalRemoveRange(Range* pRange) {
  ranges_.erase(pRange);
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
  // Note: We should scan range backward to terminate faster.
  for (auto* pRunner : ranges_) {
    if (pRunner->m_lStart > lPosn) {
      pRunner->m_lStart += iDelta;
      if (pRunner->m_lStart < lPosn)
        pRunner->m_lStart = lPosn;
      else if (pRunner->m_lStart > GetEnd())
        pRunner->m_lStart = GetEnd();
    }

    if (pRunner->m_lEnd > lPosn) {
      pRunner->m_lEnd += iDelta;
      if (pRunner->m_lEnd < lPosn)
        pRunner->m_lEnd = lPosn;
      else if (pRunner->m_lEnd > GetEnd())
        pRunner->m_lEnd = GetEnd();
    }
  }

  // Remove empty interval
  if (iDelta < 0) {
    auto const lBufEnd1 = GetEnd() + 1;

    // Note: We should scan interval backward to terminate faster.
    EnumInterval oEnum(this);
    while (!oEnum.AtEnd()) {
      auto const pRunner = oEnum.Get();
      oEnum.Next();

      auto lStart = pRunner->m_lStart;
      auto lEnd = pRunner->m_lEnd;

      if (lEnd <= lPosn)
        continue;

      if (lStart > lPosn) {
        lStart += iDelta;
        if (lStart < lPosn)
          lStart = lPosn;
        else if (lStart > lBufEnd1)
          lStart = lBufEnd1;
      }

      if (lEnd > lPosn) {
        lEnd += iDelta;
        if (lEnd < lPosn)
          lEnd = lPosn;
        else if (lEnd > lBufEnd1)
          lEnd = lBufEnd1;
      }

      if (lStart == lEnd) {
        m_oIntervals.Delete(pRunner);
        m_oIntervalTree.Delete(pRunner);

        #if DEBUG_INTERVAL
          DEBUG_PRINTF("destroyObject: interval [%d,%d] @ posn=%d%d\n",
              pRunner->m_lStart, pRunner->m_lEnd, lPosn, iDelta );
        #endif
        destroyObject(pRunner);
      }
    }
  }

  // Relocate intervals
  {
    Posn lBufEnd1 = GetEnd() + 1;
    foreach (EnumInterval, oEnum, this) {
      auto const pRunner = oEnum.Get();

      auto lStart = pRunner->m_lStart;
      auto lEnd   = pRunner->m_lEnd;

      if (lStart > lPosn) {
        lStart += iDelta;
        if (lStart < lPosn)
          lStart = lPosn;
        else if (lStart > lBufEnd1)
          lStart = lBufEnd1;
      }

      if (lEnd > lPosn) {
        lEnd += iDelta;
        if (lEnd < lPosn)
          lEnd = lPosn;
        else if (lEnd > lBufEnd1)
          lEnd = lBufEnd1;
      }

      DCHECK_LT(lStart, lEnd);
      pRunner->m_lStart = lStart;
      pRunner->m_lEnd   = lEnd;
    }
  }

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
