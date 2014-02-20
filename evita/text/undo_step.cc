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
UndoStep::UndoStep() : m_pNext(nullptr), m_pPrev(nullptr) {
}

UndoStep::~UndoStep() {
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
DeleteUndoStep::DeleteUndoStep(UndoStack* undo_stack, Posn start, Posn end)
    : TextUndoStep(start, end),
      text_(std::move(undo_stack->GetBuffer()->GetText(start, end))) {
}

DeleteUndoStep::~DeleteUndoStep() {
}

bool DeleteUndoStep::Merge(UndoStack* undo_stack, Posn start, Posn end) {
  auto const buffer = undo_stack->GetBuffer();
  if (buffer->GetCharAt(start) == 0x0A)
    return false;

  if (buffer->GetCharAt(end - 1) == 0x0A)
    return false;

  // For [Backspace] key
  // 1. abc|
  // 2. ab|
  if (m_lStart == end) {
    text_ = buffer->GetText(start, end) + text_;
    m_lStart = start;
    return true;
  }

  // For [Delete] key
  // 1. a|bc
  // 2. a|c
  if (m_lStart == start) {
    text_ += buffer->GetText(start, end);
    m_lEnd += end - start;
    return true;
  }

  return false;
}

// UndoStep
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
  pBuffer->Insert(m_lStart, text_.data(), static_cast<Count>(text_.length()));
  pBuffer->IncCharTick(1);
}

void DeleteUndoStep::Undo(Buffer* pBuffer) {
  pBuffer->Insert(m_lStart, text_.data(), static_cast<Count>(text_.length()));
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
