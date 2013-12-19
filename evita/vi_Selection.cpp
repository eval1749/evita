#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - winmain
// listener/winapp/winmain.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Selection.cpp#4 $
//
#define DEBUG_GOAL 0
#include "./vi_Selection.h"

#include "./vi_Application.h"
#include "./vi_Buffer.h"
#include "./vi_TextEditWindow.h"

//////////////////////////////////////////////////////////////////////
//
// Selection ctor
//
Selection::Selection(TextEditWindow* pWindow, Buffer* pBuffer) :
    m_fStartIsActive(false),
    m_lRestoreLineNum(0),
    m_pBuffer(pBuffer),
    m_ptGoal(-1, -1),
    m_pWindow(pWindow),
    Range(pBuffer, Kind_Selection)
{
    pBuffer->InternalAddRange(this);
} // Selection::Selection


//////////////////////////////////////////////////////////////////////
//
// Selection dtor
//
Selection::~Selection()
{
    #if DEBUG_DESTROY
        DEBUG_PRINTF("%p\n", this);
    #endif
} // Selection::~Selection


//////////////////////////////////////////////////////////////////////
//
// Selection::Blink
//
void Selection::Blink(Posn lPosn, Count nMillisecond)
{
    bool fReadOnly = GetBuffer()->IsReadOnly();
    GetBuffer()->SetReadOnly(true);
    GetWindow()->Blink(lPosn, nMillisecond);
    GetBuffer()->SetReadOnly(fReadOnly);
} // Selection::Blink


//////////////////////////////////////////////////////////////////////
//
// Selection::Collapse
//
void Selection::Collapse(CollapseWhich eCollapse)
{
    forgetGoal();
    Range::Collapse(eCollapse);
    m_fStartIsActive = eCollapse == Collapse_Start;
} // Selection::Collapse


//////////////////////////////////////////////////////////////////////
//
// Selection::Delete
//  n > 0   Delete
//  n < 0   Backspace
Count Selection::Delete(Unit eUnit, Count n)
{
    forgetGoal();
    return Range::Delete(eUnit, n);
} // Selection::DeleteAfter


//////////////////////////////////////////////////////////////////////
//
// Selection::EndKey
//  eUnit = Unit_Line | Unit_Buffer
//
Count Selection::EndKey(Unit eUnit, bool fExtend)
{
    if (IsStartActive())
    {
        Collapse(Collapse_End);
    }

    Count iEndDelta = EndOf(eUnit, fExtend);

    if (Unit_Line != eUnit)
    {
        return iEndDelta;
    }

    Posn lLineEnd = GetEnd();
    Count cTrailingSpaces = MoveEndWhile(L" \t", Count_Backward);

    // Selection.End was end of line before MoveWhile.
    if (0 == iEndDelta)
    {
        return cTrailingSpaces;
    }

    // Selection.End was in middle of line.
    if (cTrailingSpaces != iEndDelta)
    {
        return iEndDelta;
    }

    // Selection.End was at end of non-whitespaces.
    if (fExtend)
    {
        SetEnd(lLineEnd);
    }
    else
    {
        SetRange(lLineEnd, lLineEnd);
    }

    return iEndDelta - cTrailingSpaces;
} // Selection::EndKey


//////////////////////////////////////////////////////////////////////
//
// Selection::EndOf
//
Count Selection::EndOf(Unit eUnit, bool fExtend)
{
    forgetGoal();

    Posn lEnd = GetEnd();

    Posn lNew;
    switch (eUnit)
    {
    case Unit_Window:
        lNew = GetWindow()->GetEnd();
        break;

    case Unit_Line:
        lNew = GetWindow()->EndOfLine(GetActivePosn());
        break;

    default:
        return Range::EndOf(eUnit, fExtend);
    } // switch unit

    if (fExtend)
    {
        if (! IsStartActive())
        {
            SetEnd(lNew);
        }
        else if (isSingleLine())
        {
            SetEnd(lNew);
            SetStartIsActive(false);
        }
        else
        {
            SetStart(lNew);
        }
    }
    else
    {
        SetRange(lNew, lNew);
    }

    return lNew - lEnd;
} // Selection::EndOf


//////////////////////////////////////////////////////////////////////
//
// Selection::forgetGoal
//
void Selection::forgetGoal()
{
    m_ptGoal.x = m_ptGoal.y = -1;
} // Selection::forgetGoal


//////////////////////////////////////////////////////////////////////
//
// Selection::GetType
//
SelectionType Selection::GetType() const
{
    return GetStart() == GetEnd() ?
        Selection_None :
        Selection_Normal;
} // Selection::GetType


//////////////////////////////////////////////////////////////////////
//
// Selection::HomeKey
//  eUnit   : Unit_Line | Unit_Buffer
//  fExtend : bool => move start only if fExtend.
//
//  Returns number of characters moved.
//


Count Selection::HomeKey(Unit eUnit, bool fExtend)
{
    if (! IsStartActive())
    {
        Collapse(Collapse_Start);
    }

    Count iStartDelta = StartOf(eUnit, fExtend);

    if (Unit_Line != eUnit)
    {
        return iStartDelta;
    }

    Posn lLineStart = GetStart();
    Count cLeadingSpaces;
    if (fExtend)
    {
        cLeadingSpaces = MoveStartWhile(L" \t", Count_Forward);
    }
    else
    {
        cLeadingSpaces = MoveWhile(L" \t", Count_Forward);
    }

    // Selection.Start was at start of line before MoveWhile.
    if (0 == iStartDelta)
    {
        return cLeadingSpaces;
    }

    // Selection.Start was in middle of line.
    if (cLeadingSpaces != iStartDelta)
    {
        return iStartDelta;
    }

    // Selection.Start was at start of non-whitespaces.
    if (fExtend)
    {
        SetStart(lLineStart);
    }
    else
    {
        SetRange(lLineStart, lLineStart);
    }

    return iStartDelta - cLeadingSpaces;
} // Selection::HomeKey


//////////////////////////////////////////////////////////////////////
//
// Selection::isSingleLine
//
bool Selection::isSingleLine() const
{
    Posn lStart = GetWindow()->StartOfLine(GetStart());
    Posn lEnd   = GetWindow()->StartOfLine(GetEnd());
    return lStart == lEnd;
} // Selection::isSingleLine


//////////////////////////////////////////////////////////////////////
//
// Selection::moveAux
//
Count Selection::moveAux(Unit eUnit, Count n, bool fExtend)
{
    Posn lActive;
    if (fExtend)
    {
        // Extend from active position.
        lActive = GetActivePosn();
    }
    else if (GetEnd() == GetStart())
    {
        // Move insertion position.
        lActive = GetStart();
    }
    else
    {
        // Move from specified direction end.
        lActive = n > 0 ? GetEnd() : GetStart();

        // Note: We make selection collapsed instead of moving end.
        if (eUnit == Unit_Char)
        {
            if (n == 0) return 0;
            MoveTo(lActive, false);
            // Note: We count one for degenerating selection.
            return 1;
        }
    } // if

    Count k = GetWindow()->ComputeMotion(eUnit, n, m_ptGoal, &lActive);
    MoveTo(lActive, fExtend);
    return k;
} // Selection::moveAux


//////////////////////////////////////////////////////////////////////
//
// Selection::MoveDown
//  Unit        | Key Combination
//  ------------+----------------
//  Line        | Down
//  Paragraph   | Ctrl+Down
/// Screen      | PageDown
//  Window      | Ctrl+PageDown
Count Selection::MoveDown(Unit eUnit, Count n, bool fExtend)
{
    updateGoal();
    return moveAux(eUnit, n, fExtend);
} // Selection::MoveDown


//////////////////////////////////////////////////////////////////////
//
// Selection::MoveLeft
//  Unit        | Key Combination
//  ------------+----------------
//  Char        | Left
/// Word        | Ctrl+Left
Count Selection::MoveLeft(Unit eUnit, Count n, bool fExtend)
{
    forgetGoal();
    return moveAux(eUnit, -n, fExtend);
} // Selection::MoveLeft


//////////////////////////////////////////////////////////////////////
//
// Selection::MoveRight
//  Unit        | Key Combination
//  ------------+----------------
//  Char        | Right
/// Word        | Ctrl+Right
//
// Note: If selection isn't nondegenerate and fExtend is false, selection
// becomes degenerate.
Count Selection::MoveRight(Unit eUnit, Count n, bool fExtend)
{
    forgetGoal();
    return moveAux(eUnit, n, fExtend);
} // Selection::MoveRight


//////////////////////////////////////////////////////////////////////
//
// Selection::MoveTo
//
// Description:
//  Sets start and end by specified position.
//
//  This method is used for implementing mouse click.
//
void Selection::MoveTo(Posn lPosn, bool fExtend)
{
    if (! fExtend)
    {
        Range::SetRange(lPosn, lPosn);
    }
    else
    {
        Range::SetRange(lPosn, m_fStartIsActive ? GetEnd() : GetStart());
        m_fStartIsActive = lPosn <= GetStart();
    }
} // Selection::MoveTo


//////////////////////////////////////////////////////////////////////
//
// Selection::MoveUp
//
//  Unit        | Key Combination
//  ------------+----------------
//  Line        | Up
//  Paragraph   | Ctrl+Up
/// Screen      | PageUp
//  Window      | Ctrl+PageUp
Count Selection::MoveUp(Unit eUnit, Count n, bool fExtend)
{
    updateGoal();
    return moveAux(eUnit, -n, fExtend);
} // Selection::MoveUp


//////////////////////////////////////////////////////////////////////
//
// Selection::RestoreForReload
//
void Selection::RestoreForReload()
{
    SetRange(0, 0);
    Move(Unit_Paragraph, m_lRestoreLineNum);
} // Selection::RestoreForReload


//////////////////////////////////////////////////////////////////////
//
// Selection::PrepareForReload
//
void Selection::PrepareForReload()
{
    Information oInfo;
    GetInformation(&oInfo);
    m_lRestoreLineNum = oInfo.m_lLineNum;
} // Selection::PrepareForReload


//////////////////////////////////////////////////////////////////////
//
// Selection::SetText
//
void Selection::SetText(const char16* pwsz)
{
    SetText(pwsz, ::lstrlenW(pwsz));
} // Selection::SetText


//////////////////////////////////////////////////////////////////////
//
// Selection::SetText
//
void Selection::SetText(const char16* pwch, int cwch)
{
    forgetGoal();
    Range::SetText(pwch, cwch);
} // Selection::SetText


void Selection::SetEnd(Posn p)
    { forgetGoal(); Range::SetEnd(p); }

void Selection::SetRange(Posn s, Posn e)
    { forgetGoal(); Range::SetRange(s, e); }

void Selection::SetRange(Range* p)
    { forgetGoal(); Range::SetRange(p); }

void Selection::SetStart(Posn p)
    { forgetGoal(); Range::SetStart(p); }


//////////////////////////////////////////////////////////////////////
//
// Selection::StartOf
//
Count Selection::StartOf(Unit eUnit, bool fExtend)
{
    forgetGoal();

    Posn lStart = GetStart();

    Posn lNew;
    switch (eUnit)
    {
    case Unit_Line:
        lNew = GetWindow()->StartOfLine(GetActivePosn());
        break;

    case Unit_Window:
        lNew = GetWindow()->GetStart();
        break;

    default:
        return Range::StartOf(eUnit, fExtend);
    } // switch unit

    if (fExtend)
    {
        if (IsStartActive())
        {
            SetStart(lNew);
        }
        else if (isSingleLine())
        {
            SetStart(lNew);
            SetStartIsActive(true);
        }
        else
        {
            SetEnd(lNew);
        }
    }
    else
    {
        SetRange(lNew, lNew);
    }

    return lStart - lNew;
} // Selection::StartOf


//////////////////////////////////////////////////////////////////////
//
// Selection::TypeChar
//
void Selection::TypeChar(char16 wch, Count k)
{
    Buffer* pBuffer = GetBuffer();
    if (Selection_None == GetType())
    {
        pBuffer->Insert(GetStart(), wch, k);
    }
    else
    {
        Edit::UndoBlock oUndo(pBuffer, L"Selection.TypeChar");
        SetText(NULL, 0);
        pBuffer->Insert(GetStart(), wch, k);
    } // if

    MoveRight(Unit_Char, k);
} // Selection::TypeChar


//////////////////////////////////////////////////////////////////////
//
// Selection::TypeEnter
//
void Selection::TypeEnter(Count k)
{
    if (k <= 0)
    {
        return;
    }

    // Get leading whitespaces
    StringResult oSpaces;
    {
        Range oRange(this);
        oRange.StartOf(Unit_Paragraph);
        oRange.MoveEndWhile(L" \t");
        oRange.GetText(&oSpaces);
    }

    Edit::UndoBlock oUndo(GetBuffer(), L"Selection.TypeEnter");

    // Delete trailing whitespaces
    {
        Range oRange(this);
        oRange.MoveStartWhile(L" \t", Count_Backward);
        oRange.SetText(NULL, 0);
    }

    TypeChar(0x0A, k);
    MoveEndWhile(L" \t");
    SetText(oSpaces);
    Collapse(Collapse_End);
} // Selection::TypeEnter


//////////////////////////////////////////////////////////////////////
//
// Selection::TypeText
//
void Selection::TypeText(const char16* pwch, Count cwch)
{
    SetText(pwch, cwch);
    Collapse(Collapse_End);
} // Selection::TypeText


//////////////////////////////////////////////////////////////////////
//
// Selection::updateGoal
//
bool Selection::updateGoal()
{
    if (m_ptGoal.x >= 0)
    {
        return false;
    }

    const auto rect = GetWindow()->MapPosnToPoint(GetActivePosn());
    if (rect)
      m_ptGoal = rect.left_top();

    #if DEBUG_GOAL
        DEBUG_PRINTF("%d %p (%d,%d)\n",
            ::GetTickCount(), this, static_cast<int>(m_ptGoal.x * 100),
            static_cast<int>(m_ptGoal.y * 100));
    #endif // DEBUG_GOAL

    return true;
} // Selection::updateGoal
