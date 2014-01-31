#include "precomp.h"
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Selection.h"

#include "base/strings/string16.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/vi_TextEditWindow.h"

Selection::Selection(const text::Range& range)
    : Range(static_cast<Buffer*>(range.GetBuffer()), Kind_Selection),
      m_fStartIsActive(false),
      m_lRestoreLineNum(0),
      m_pBuffer(static_cast<Buffer*>(range.GetBuffer())),
      m_pWindow(nullptr) {
  m_pBuffer->InternalAddRange(this);
  SetRange(&range);
}

Selection::~Selection() {
}

void Selection::Collapse(CollapseWhich eCollapse) {
  Range::Collapse(eCollapse);
  m_fStartIsActive = eCollapse == Collapse_Start;
}

Selection* Selection::Create(const text::Range& range) {
  return new(range.GetBuffer()->GetHeap()) Selection(range);
}

void Selection::RestoreForReload() {
  SetRange(0, 0);
  Move(Unit_Line, m_lRestoreLineNum);
}

void Selection::PrepareForReload() {
  Information oInfo;
  GetInformation(&oInfo);
  m_lRestoreLineNum = oInfo.m_lLineNum;
}

void Selection::SetStartIsActive(bool new_start_is_active) {
  m_fStartIsActive = new_start_is_active;
}

void Selection::TypeChar(char16 wch, Count k) {
  auto const pBuffer = GetBuffer();
  if (GetStart() == GetEnd()) {
    pBuffer->Insert(GetStart(), wch, k);
  } else {
    text::UndoBlock oUndo(pBuffer, L"Selection.TypeChar");
    SetText(base::string16());
    pBuffer->Insert(GetStart(), wch, k);
  }

  if (GetEnd() == GetStart())
    Move(Unit_Char, k);
  else
    Collapse(Collapse_End);
}
