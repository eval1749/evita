#include "precomp.h"
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Selection.h"

#include "base/strings/string16.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/vi_TextEditWindow.h"

Selection::Selection(TextEditWindow* pWindow, Buffer* pBuffer) :
    m_fStartIsActive(false),
    m_lRestoreLineNum(0),
    m_pBuffer(pBuffer),
    m_ptGoal(-1, -1),
    m_pWindow(pWindow),
    Range(pBuffer, Kind_Selection) {
    pBuffer->InternalAddRange(this);
}

Selection::Selection(const text::Range& range)
    : Selection(nullptr, static_cast<Buffer*>(range.GetBuffer())) {
  SetRange(&range);
}

Selection::~Selection() {
}

void Selection::Blink(Posn lPosn, Count nMillisecond) {
  bool fReadOnly = GetBuffer()->IsReadOnly();
  GetBuffer()->SetReadOnly(true);
  GetWindow()->Blink(lPosn, nMillisecond);
  GetBuffer()->SetReadOnly(fReadOnly);
}

void Selection::Collapse(CollapseWhich eCollapse) {
  forgetGoal();
  Range::Collapse(eCollapse);
  m_fStartIsActive = eCollapse == Collapse_Start;
}

Selection* Selection::Create(const text::Range& range) {
  return new(range.GetBuffer()->GetHeap()) Selection(range);
}

void Selection::forgetGoal() {
    m_ptGoal.x = m_ptGoal.y = -1;
}

SelectionType Selection::GetType() const {
  return GetStart() == GetEnd() ? Selection_None : Selection_Normal;
}

Count Selection::moveAux(Unit eUnit, Count n, bool fExtend) {
  Posn lActive;
  if (fExtend) {
      // Extend from active position.
      lActive = GetActivePosn();
  } else if (GetEnd() == GetStart()) {
      // Move insertion position.
      lActive = GetStart();
  } else {
    // Move from specified direction end.
    lActive = n > 0 ? GetEnd() : GetStart();

    // Note: We make selection collapsed instead of moving end.
    if (eUnit == Unit_Char) {
        if (!n)
          return 0;
        MoveTo(lActive, false);
        // Note: We count one for degenerating selection.
        return 1;
    }
  }

  auto const k = GetWindow()->ComputeMotion(eUnit, n, m_ptGoal, &lActive);
  MoveTo(lActive, fExtend);
  return k;
}

//  Unit        | Key Combination
//  ------------+----------------
//  Char        | Right
/// Word        | Ctrl+Right
//
// Note: If selection isn't nondegenerate and fExtend is false, selection
// becomes degenerate.
Count Selection::MoveRight(Unit eUnit, Count n, bool fExtend) {
  forgetGoal();
  return moveAux(eUnit, n, fExtend);
}

void Selection::MoveTo(Posn lPosn, bool fExtend) {
  if (!fExtend) {
    Range::SetRange(lPosn, lPosn);
  } else {
    Range::SetRange(lPosn, m_fStartIsActive ? GetEnd() : GetStart());
    m_fStartIsActive = lPosn <= GetStart();
  }
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

void Selection::SetText(const char16* pwsz) {
  SetText(pwsz, ::lstrlenW(pwsz));
}

void Selection::SetText(const char16* pwch, int cwch) {
  forgetGoal();
  Range::SetText(base::string16(pwch, static_cast<size_t>(cwch)));
}

void Selection::SetEnd(Posn p) {
  forgetGoal();
  Range::SetEnd(p);
}

void Selection::SetRange(Posn s, Posn e) {
  forgetGoal();
  Range::SetRange(s, e);
}

void Selection::SetRange(const Range* p) {
  forgetGoal();
  Range::SetRange(p);
}

void Selection::SetStart(Posn p) {
  forgetGoal();
  Range::SetStart(p);
}

void Selection::SetStartIsActive(bool new_start_is_active) {
  // TODO(yosi) We should remove |forgotGoal()| call, once we implement new
  // goal point tracking. Calling |forgotGoal()| here is temporary solution.
  forgetGoal();
  m_fStartIsActive = new_start_is_active;
}

void Selection::TypeChar(char16 wch, Count k) {
  auto const pBuffer = GetBuffer();
  if (Selection_None == GetType()) {
    pBuffer->Insert(GetStart(), wch, k);
  } else {
    text::UndoBlock oUndo(pBuffer, L"Selection.TypeChar");
    SetText(nullptr, 0);
    pBuffer->Insert(GetStart(), wch, k);
  }

  if (GetEnd() == GetStart())
    Move(Unit_Char, k);
  else
    Collapse(Collapse_End);
}

bool Selection::updateGoal() {
  if (m_ptGoal.x >= 0)
    return false;

  const auto rect = GetWindow()->MapPosnToPoint(GetActivePosn());
  if (rect)
    m_ptGoal = rect.left_top();
  return true;
}
