// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/undo_step.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/undo_stack.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// UndoStep
//
void* UndoStep::operator new(size_t size, UndoStack* undo_stack) {
  return undo_stack->Alloc(size);
}

void UndoStep::operator delete(void*) {
  NOTREACHED();
}

UndoStep::UndoStep() : m_pNext(nullptr), m_pPrev(nullptr) {
}

UndoStep::~UndoStep() {
}

void UndoStep::Discard(UndoStack*) {
}

void UndoStep::Redo(Buffer*) {
}

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
TextUndoStep::TextUndoStep(Posn lStart, Posn lEnd)
    : m_lEnd(lEnd),
      m_lStart(lStart) {
  DCHECK_LE(m_lStart, m_lEnd);
}

TextUndoStep::~TextUndoStep() {
}

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//

BeginUndoStep::BeginUndoStep(const base::string16& name) : name_(name) {
}

BeginUndoStep::~BeginUndoStep() {
}

// UndoStep
Posn BeginUndoStep::GetAfterRedo() const {
  return m_pNext->GetAfterRedo();
}

Posn BeginUndoStep::GetAfterUndo() const {
  return m_pNext->GetAfterUndo();
}

Posn BeginUndoStep::GetBeforeRedo() const {
  return m_pNext->GetBeforeRedo();
}

Posn BeginUndoStep::GetBeforeUndo() const {
  return m_pNext->GetBeforeUndo();
}

void BeginUndoStep::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->EndUndoGroup(name_);
}

//////////////////////////////////////////////////////////////////////
//
// DeleteUndoStep
//
class DeleteUndoStep::Chars {
  friend class DeleteUndoStep;

  private: Count m_cwch;
  private: Chars* m_pNext;

  public: void* operator new(size_t cb, UndoStack* pUndo, Posn lStart,
                             Posn lEnd) {
    return pUndo->Alloc(cb + sizeof(char16) * (lEnd - lStart));
  }

  public: Count GetLength() const { return m_cwch; }

  public: const char16* GetString() const {
    return reinterpret_cast<const char16*>(this + 1);
  }

  public: Chars(UndoStack* pUndo, Posn lStart, Posn lEnd)
      : m_cwch(lEnd - lStart),
        m_pNext(nullptr) {
    auto const pBuffer = pUndo->GetBuffer();
    auto pwch = const_cast<char16*>(GetString());
    for (Posn lPosn = lStart; lPosn < lEnd; lPosn++) {
      *pwch++ = pBuffer->GetCharAt(lPosn);
    }
  }

  DISALLOW_COPY_AND_ASSIGN(Chars);
};

class DeleteUndoStep::EnumChars {
  private: Chars* m_pRunner;

  public: EnumChars(const DeleteUndoStep* p) : m_pRunner(p->m_pFirst) {
  }

  public: bool AtEnd() const { return nullptr == m_pRunner; }

  public: Chars* Get() const {
    DCHECK(!AtEnd());
    return m_pRunner;
  }

  public: void Next() {
    DCHECK(!AtEnd());
    m_pRunner = m_pRunner->m_pNext;
  }

  DISALLOW_COPY_AND_ASSIGN(EnumChars);
};

DeleteUndoStep::DeleteUndoStep(UndoStack* pUndo, Posn lStart, Posn lEnd)
    : TextUndoStep(lStart, lEnd) {
  m_pFirst = createChars(pUndo, lStart, lEnd);
  m_pLast = m_pFirst;
}

DeleteUndoStep::~DeleteUndoStep() {
}

DeleteUndoStep::Chars* DeleteUndoStep::createChars(UndoStack* pUndo,
                                               Posn lStart, Posn lEnd) {
  return new(pUndo, lStart, lEnd) Chars(pUndo, lStart, lEnd);
}

void DeleteUndoStep::insertChars(Buffer* pBuffer) const {
  auto lPosn = m_lStart;
  foreach (EnumChars, oEnum, this) {
    auto const pChars = oEnum.Get();
    pBuffer->Insert(lPosn, pChars->GetString(), pChars->GetLength());
    lPosn += pChars->GetLength();
  }
}

bool DeleteUndoStep::Merge(UndoStack* pUndo, Posn lStart, Posn lEnd) {
  if (pUndo->GetBuffer()->GetCharAt(lStart) == 0x0A)
    return false;

  if (pUndo->GetBuffer()->GetCharAt(lEnd - 1) == 0x0A)
    return false;

  // For [Backspace] key
  // 1. abc|
  // 2. ab|
  if (m_lStart == lEnd) {
    auto const pChars = createChars(pUndo, lStart, lEnd);
    pChars->m_pNext = m_pFirst;
    m_pFirst = pChars;
    m_lStart = lStart;
    return true;
  }

  // For [Delete] key
  // 1. a|bc
  // 2. a|c
  if (m_lStart == lStart) {
    auto const pChars = createChars(pUndo, lStart, lEnd);
    m_pLast->m_pNext = pChars;
    m_pLast = pChars;
    m_lEnd += lEnd - lStart;
    return true;
  }

  return false;
}

// UndoStep
void DeleteUndoStep::Discard(UndoStack* pUndo) {
  EnumChars oEnum(this);
  while (!oEnum.AtEnd()) {
      Chars* pChars = oEnum.Get();
      oEnum.Next();
      pUndo->Free(pChars);
  }
}

Posn DeleteUndoStep::GetAfterRedo() const {
  return m_lEnd;
}

Posn DeleteUndoStep::GetAfterUndo() const {
  return m_lEnd;
}

Posn DeleteUndoStep::GetBeforeRedo() const {
  return m_lStart;
}

Posn DeleteUndoStep::GetBeforeUndo() const {
  return m_lStart;
}

void DeleteUndoStep::Redo(Buffer* pBuffer) {
  insertChars(pBuffer);
  pBuffer->IncCharTick(1);
}

void DeleteUndoStep::Undo(Buffer* pBuffer) {
  insertChars(pBuffer);
  pBuffer->GetUndo()->PushInsertText(m_lStart, m_lEnd);
  pBuffer->IncCharTick(-1);
}

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
EndUndoStep::EndUndoStep(const base::string16& name) : name_(name) {
}

EndUndoStep::~EndUndoStep() {
}

bool EndUndoStep::CanMerge(const base::string16& name) const {
  return name_ == name && '*' == name[0];
}

// UndoStep
Posn EndUndoStep::GetAfterRedo() const {
  return m_pPrev->GetAfterRedo();
}

Posn EndUndoStep::GetAfterUndo() const {
  return m_pPrev->GetAfterUndo();
}

Posn EndUndoStep::GetBeforeRedo() const {
  return m_pPrev->GetBeforeRedo();
}

Posn EndUndoStep::GetBeforeUndo() const {
  return m_pPrev->GetBeforeUndo();
}

void EndUndoStep::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->BeginUndoGroup(name_);
}

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
InsertUndoStep::InsertUndoStep(Posn lStart, Posn lEnd)
    : TextUndoStep(lStart, lEnd) {
}

InsertUndoStep::~InsertUndoStep() {
}

bool InsertUndoStep::Merge(Buffer* pBuffer, Posn lStart, Posn lEnd) {
  if (m_lEnd == lStart) {
    // [last][new]
    if (pBuffer->GetCharAt(m_lEnd - 1) != 0x0A) {
      m_lEnd = lEnd;
      return true;
    }
  } else if (m_lStart == lEnd) {
    // [new][last]
    if (pBuffer->GetCharAt(lEnd - 1) != 0x0A) {
      m_lStart = lStart;
      return true;
    }
  }
  return false;
}

// UndoStep
Posn InsertUndoStep::GetAfterRedo() const {
  return m_lStart;
}

Posn InsertUndoStep::GetAfterUndo() const {
  return m_lStart;
}

Posn InsertUndoStep::GetBeforeRedo() const {
  return m_lEnd;
}

Posn InsertUndoStep::GetBeforeUndo() const {
  return m_lEnd;
}

void InsertUndoStep::Redo(Buffer* pBuffer) {
  pBuffer->Delete(m_lStart, m_lEnd);
  pBuffer->IncCharTick(1);
}

void InsertUndoStep::Undo(Buffer* pBuffer) {
  pBuffer->GetUndo()->PushDeleteText(m_lStart, m_lEnd);
  pBuffer->Delete(m_lStart, m_lEnd);
  pBuffer->IncCharTick(-1);
}

}  // namespace text
