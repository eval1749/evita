// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/range_set.h"
#include "evita/text/undo_stack.h"

// Smart handle for HGLOBAL
template<class T>
class Global {
  private: HGLOBAL m_h;
  private: T* m_p;

  public: Global() : m_h(nullptr), m_p(nullptr) {
  }

  public: ~Global() {
    if (m_h) {
      if (m_p)
        ::GlobalUnlock(m_h);
      ::GlobalFree(m_h);
    }
  }

  public: operator HANDLE() { return reinterpret_cast<HANDLE>(m_h); }

  public: bool Alloc(size_t cb) {
    DCHECK(!m_h);
    m_h = ::GlobalAlloc(GMEM_MOVEABLE, cb);
    return m_h != nullptr;
  }

  public: void Detach() {
    DCHECK(!m_p);
    DCHECK(m_h);
    m_h = nullptr;
  }

  public: T* Lock() {
    if (!m_h)
      return nullptr;
    return m_p = reinterpret_cast<T*>(::GlobalLock(m_h));
  }

  public: void Unlock() {
    if (!m_h || !m_p)
        return;
    m_p = nullptr;
    ::GlobalUnlock(m_h);
  }
};

// Smart handle for Windows clipboard
class Clipboard {
  private: bool m_fSucceeded;
  private: mutable HANDLE m_hGlobal;

  public: Clipboard()
      : m_fSucceeded(::OpenClipboard(nullptr)), m_hGlobal(nullptr) {
  }

  public: ~Clipboard() {
    if (m_hGlobal)
      ::GlobalUnlock(m_hGlobal);
    if (m_fSucceeded)
      ::CloseClipboard();
  }

  public: bool Empty() { return ::EmptyClipboard(); }

  public: char16* GetText() const {
    m_hGlobal = ::GetClipboardData(CF_UNICODETEXT);
    if (!m_hGlobal)
      return nullptr;
    return reinterpret_cast<char16*>(::GlobalLock(m_hGlobal));
  }

  public: bool HasFormat(uint uFormat) const {
    return ::IsClipboardFormatAvailable(uFormat);
  }

  public: bool IsOpen() const { return m_fSucceeded; }

  public: bool Set(HANDLE h) {
    return ::SetClipboardData(CF_UNICODETEXT, h);
  }
};

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

Count Range::Copy() {
  if (start_ == end_)
    return 0;

  Count cwch = 0;
  for (Posn lPosn = start_; lPosn < end_; lPosn += 1) {
    auto const wch = buffer_->GetCharAt(lPosn);
    if (wch == 0x0A)
      ++cwch;
    ++cwch;
  }

  auto const cb = sizeof(char16) * (cwch + 1);

  Global<char16> oGlobal;
  if (!oGlobal.Alloc(cb))
    return 0;

  {
    auto pwch = oGlobal.Lock();
    if (!pwch)
      return 0;
    for (Posn lPosn = start_; lPosn < end_; lPosn += 1) {
      auto const wch = buffer_->GetCharAt(lPosn);
      if (wch == 0x0A)
        *pwch++ = 0x0D;
      *pwch++ = wch;
    }
    *pwch = 0;
    oGlobal.Unlock();
  }

  Clipboard oClipboard;
  if (!oClipboard.IsOpen())
    return 0;
  if (!oClipboard.Empty())
    return 0;
  if (!oClipboard.Set(oGlobal))
    return 0;

  oGlobal.Detach();
  return cwch;
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

void Range::Paste() {
  if (buffer_->IsReadOnly())
    return;

  Clipboard oClipboard;
  if (!oClipboard.IsOpen())
    return;

  char16* pwsz = oClipboard.GetText();
  if (!pwsz)
    return;

  if (!*pwsz)
    return;

  UndoBlock oUndo(this, L"Range.Paste");

  buffer_->Delete(start_, end_);

  auto lPosn = start_;
  auto pwchStart = pwsz;
  enum { Start, Normal, Cr } eState = Start;
  while (*pwsz) {
    switch (eState) {
      case Normal:
        if (0x0D == *pwsz)
          eState = Cr;
        break;

      case Cr:
        switch (*pwsz) {
          case 0x0A: {
            pwsz[-1] = 0x0A;
            Count k = static_cast<Count>(pwsz - pwchStart);
            buffer_->Insert(lPosn, pwchStart, k);
            lPosn += k;
            eState = Start;
            pwchStart = pwsz + 1;
            break;
        }

          case 0x0D:
            break;

          default:
            eState = Normal;
            break;
        }
        break;

      case Start:
        eState = 0x0D == *pwsz ? Cr : Normal;
        break;
    }
    ++pwsz;
  }

  if (Start != eState) {
    auto const k = static_cast<Count>(pwsz - pwchStart);
    buffer_->Insert(lPosn, pwchStart, k);
    lPosn += k;
  }

  start_ = lPosn;
  end_ = lPosn;
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
