#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Find Dialog
// listener/winapp/dlg_find.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FindDialogBox.cpp#9 $
//
#include "./vi_FindDialogBox.h"

#include "./RegexMatch.h"

#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "./vi_EditPane.h"
#include "./vi_Frame.h"
#include "./vi_Selection.h"
#include "./vi_TextEditWindow.h"

#define BEGIN_COMMAND_MAP switch (wParam) {
#define END_COMMAND_MAP } return false;

#define ON_COMMAND(mp_ctrl, mp_notify, mp_method) \
    case MAKEWPARAM(mp_ctrl, mp_notify): \
    { \
        mp_method(); \
        return true; \
    }

/// <summary>
///   Returns active selection
/// </summary>
/// <returns>
///   Active selection
/// </returns>
static Selection* getActiveSelection()
{
    EditPane* pEditPane =
        Application::instance()->GetActiveFrame()->GetActivePane()->
            DynamicCast<EditPane>();

    if (NULL == pEditPane)
    {
        return NULL;
    }

    auto const window = pEditPane->GetActiveWindow();
    if (!window)
        return nullptr;

    if (auto const text_edit_window = window->as<TextEditWindow>())
        return text_edit_window->GetSelection();

    return nullptr;
} // getActiveSelection

/// <summary>
///   Constructs Find Dialog Box with default values.
/// </summary>
FindDialogBox::FindDialogBox() :
    m_eDirection(Direction_Down),
    m_eReplaceIn(ReplaceIn_Whole)
    {}

/// <summary>
///   Clear message in active frame
/// </summary>
void FindDialogBox::clearMessage()
{
    Application::instance()->GetActiveFrame()->ShowMessage(MessageLevel_Warning);
} // FindDialogBox::clearMesage

/// <summary>
//   Handles [Find Next] button, [F3], and Shift+[F3] key
/// </summary>
void FindDialogBox::DoFind(Direction eDirection)
{
    clearMessage();

    m_eDirection = eDirection;

    SearchParameters oSearch;
    Selection* pSelection = prepareFind(&oSearch);
    if (NULL == pSelection)
    {
        // We may not have search text.
        return;
    }

    if (Direction_Up == eDirection) 
    {
        oSearch.m_rgf |= SearchFlag_Backward;
    }

    Buffer* pBuffer = pSelection->GetBuffer();

    Posn lStart = pSelection->GetStart();
    Posn lEnd   = pSelection->GetEnd();

    if (oSearch.IsWhole())
    {
        if (oSearch.IsBackward())
        {
            lStart = 0;
            lEnd   = pSelection->GetStart();
        }
        else
        {
            lStart = pSelection->GetEnd();
            lEnd   = pBuffer->GetEnd();
        }
    } // if whole

    RegexMatcher oMatcher(&oSearch, pSelection->GetBuffer(), lStart, lEnd);

    {
        int nChar;
        if (int nError = oMatcher.GetError(&nChar))
        {
            Application::instance()->GetActiveFrame()->ShowMessage(
                MessageLevel_Warning,
                IDS_BAD_REGEX,
                nChar,
                nError );
            return;
        }
    }

    if (! findFirst(&oMatcher))
    {
        reportNotFound();
        return;
    }

    pSelection->SetRange(oMatcher.GetMatched(0));
    pSelection->SetStartIsActive(oSearch.IsBackward());
    pSelection->GetWindow()->MakeSelectionVisible();

    updateUI();
} // FindDialogBox::DoFind

/// <summary>
///   Replaces region with specified case.
/// </summary>
/// <param name="pRange">A range to replace</param>
/// <param name="eCase">Case for replace</param>
static void caseReplace(Edit::Range* pRange, StringCase eCase)
{
    switch (eCase)
    {
    case StringCase_Capitalized:
        pRange->Capitalize();
        break;

    case StringCase_CapitalizedAll:
        pRange->CapitalizeAll();
        break;

    case StringCase_Lower:
        pRange->Downcase();
        break;

    case StringCase_Mixed:
    case StringCase_None:
      // Nothing to do
        break;

    case StringCase_Upper:
        pRange->Upcase();
        break;
    } // switch eCase
} // caseReplace

/// <summary>
///   Handles [Replace] and [Replace All] button click.
/// </summary>
/// <param name="nCount">
///   Specifies maximum number of replacement. For [Replace] button, this
///   value is 1.
/// </param>
void FindDialogBox::doReplace(uint nCtrl)
{
    clearMessage();

    SearchParameters oSearch;
    Selection* pSelection = prepareFind(&oSearch);
    if (NULL == pSelection)
    {
        // We may not have search text.
        return;
    }

    Edit::Buffer* pBuffer = pSelection->GetBuffer();

    Posn lStart = pSelection->GetStart();
    Posn lEnd   = pSelection->GetEnd();

    if (oSearch.IsWhole())
    {
        lStart = pBuffer->GetStart();
        lEnd   = pBuffer->GetEnd();
    } // if whole

    RegexMatcher oMatcher(&oSearch, pSelection->GetBuffer(), lStart, lEnd);

    if (! findFirst(&oMatcher))
    {
        reportNotFound();
        return;
    }

    char16 wszWith[100];
    ::GetWindowText(
        GetDlgItem(IDC_FIND_WITH),
        wszWith,
        lengthof(wszWith) );

    int cwchWith = ::lstrlenW(wszWith);

    bool fReplaceWithMeta = 0 != (oSearch.m_rgf & SearchFlag_Regex);

    Count cReplaced = 0;

    if (IDC_FIND_REPLACE == nCtrl)
    {
        Edit::Range* pRange = oMatcher.GetMatched(0);

        if (pSelection->GetStart() == pRange->GetStart() &&
            pSelection->GetEnd()   == pRange->GetEnd() )
        {
            if (oSearch.IsCasePreserve())
            {
                Edit::UndoBlock oUndo(pBuffer, L"Edit.Replace");
                StringCase eCase = pSelection->AnalyzeCase();
                oMatcher.Replace(wszWith, cwchWith, fReplaceWithMeta);
                caseReplace(pSelection, eCase);
            }
            else
            {
                oMatcher.Replace(wszWith, cwchWith, fReplaceWithMeta);
            }

            cReplaced += 1;
        }

        // Just select matched string or replaced string.
        pSelection->SetRange(pRange);
        pSelection->SetStartIsActive(false);
    }
    else
    {
        // Replace multiple matched strings
        Edit::UndoBlock oUndo(pBuffer, L"Edit.Replace");

        Edit::Range oRange(pBuffer, lStart, lEnd);

        do
        {
            Edit::Range* pRange = oMatcher.GetMatched(0);
            ASSERT(NULL != pRange);

            bool fEmptyMatch = pRange->GetStart() == pRange->GetEnd();

            if (oSearch.IsCasePreserve())
            {
                StringCase eCase = pRange->AnalyzeCase();
                oMatcher.Replace(wszWith, cwchWith, fReplaceWithMeta);
                caseReplace(pRange, eCase);
            }
            else
            {
                oMatcher.Replace(wszWith, cwchWith, fReplaceWithMeta);
            }

            cReplaced += 1;

            if (fEmptyMatch)
            {
                Posn lPosn = pRange->GetEnd();
                if (pBuffer->GetEnd() == lPosn)
                {
                    // We reach at end of buffer.
                    break;
                }

                pRange->SetEnd(lPosn + 1);
            } // if empty match

            // FIXME 2008-07-09 yosi@msn.com We should allow interrupt
            // replacing.

            pRange->Collapse(Collapse_End);
        } while (oMatcher.NextMatch());
    } // if

    Application::instance()->GetActiveFrame()->ShowMessage(
        MessageLevel_Information,
        IDS_REPLACED,
        cReplaced );

    pSelection->GetWindow()->MakeSelectionVisible();
} // FindDialogBox::doReplace

/// <summary>
///   Find the first match by executing matcher.
/// </summary>
/// <param name="pMatcher">
///   A matcher initialized from Find Dialog input elements.
/// </param>
bool FindDialogBox::findFirst(
    RegexMatcher* pMatcher )
{
    if (pMatcher->FirstMatch())
    {
        return true;
    }

    if (pMatcher->WrapMatch())
    {
        Application::instance()->ShowMessage(
            MessageLevel_Warning,
            IDS_PASSED_END );

        return true;
    }

    return false;
} // FindDialogBox::findFirst

/// <summary>
///   Handles [Cancel] button.
/// </summary>
void FindDialogBox::onCancel()
{
    clearMessage();
    ::ShowWindow(m_hwnd, SW_HIDE);
    ::SetActiveWindow(*Application::instance()->GetActiveFrame());
} // FindDialogBox::onCancel

/// <summary>
///   Dispatch WM_COMMAND to controls.
/// </summary>
/// <param name="wParam">
///   wParam of window message
/// </param>
/// <param name="lParam">
///   lParam of window message
/// </param>
bool FindDialogBox::onCommand(WPARAM wParam, LPARAM)
{
    DEBUG_PRINTF("ctrlid=%d notify=%x\n", LOWORD(wParam), HIWORD(wParam));

    BEGIN_COMMAND_MAP
        ON_COMMAND(IDOK,     BN_CLICKED, onOk)      // [Entery] key
        ON_COMMAND(IDCANCEL, BN_CLICKED, onCancel)  // [Esc] key

        ON_COMMAND(IDC_FIND_NEXT,        BN_CLICKED,        onFindNext)
        ON_COMMAND(IDC_FIND_PREVIOUS,    BN_CLICKED,        onFindPrevious)
        ON_COMMAND(IDC_FIND_REPLACE,     BN_CLICKED,        onReplaceOne)
        ON_COMMAND(IDC_FIND_REPLACE_ALL, BN_CLICKED,        onReplaceAll)
        ON_COMMAND(IDC_FIND_WHAT,        CBN_EDITCHANGE,    updateUI)
        ON_COMMAND(IDC_FIND_WITH,        CBN_EDITCHANGE,    updateUI)
    END_COMMAND_MAP
} // FindDialogBox::onCommand


/// <summary>
///   Initialize Find dialog box. This method is called at WM_INITDIALOG.
/// </summary>
bool FindDialogBox::onInitDialog()
{
    auto const dwExStyle = ::GetWindowLong(*this, GWL_EXSTYLE) |
        WS_EX_LAYERED;
    ::SetWindowLong(*this, GWL_EXSTYLE, dwExStyle);

    // FIXME 2007-08-20 yosi@msn.com We should get default value of
    // find dialog transparency.
    ::SetLayeredWindowAttributes(
        *this,
        RGB(0, 0, 0),
        80 * 255 / 100,
        LWA_ALPHA );

    SetCheckBox(IDC_FIND_EXACT, true);

    // FIXME 2007-08-20 yosi@msn.com We should get default value of
    // case-preserving replace.
    SetCheckBox(IDC_FIND_PRESERVE, true);

    updateUI(true);

    return true;
} // FindDialogBox::onInitDialog

/// <summary>
///   Dispatch windows message.
/// </summary>
INT_PTR FindDialogBox::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //DEBUG_PRINTF("%p uMsg=0x%04x\n", this, uMsg);
    switch (uMsg)
    {
    case WM_ACTIVATE:
        DEBUG_PRINTF("WM_ACTIVATE %p wParam=%d\n", this, wParam);

        if (WA_INACTIVE == wParam)
        {
            //::SetActiveWindow(*Application::instance()->GetActiveFrame());
            //::SetFocus(*Application::instance()->GetActiveFrame());
            return FALSE;
        }

        updateUI(true);
        ::SetFocus(GetDlgItem(IDC_FIND_WHAT));
        return TRUE;

    case WM_WINDOWPOSCHANGED:
    {
        WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lParam);
        if (wp->flags & SWP_HIDEWINDOW)
        {
            DEBUG_PRINTF("WM_WINDOWPOSCHANGED: %d %p SWP_HIDEWINDOW\n",
                ::GetTickCount(), this );
            //::SetActiveWindow(*Application::instance()->GetActiveFrame());
            Application::instance()->GetActiveFrame()->GetActivePane()->SetFocus();
            return TRUE;
        }
        break;
    } // WM_WINDOWPOSCHANGED
    } // swtich message

    return DialogBox::onMessage(uMsg, wParam, lParam);
} // FindDialogBox::onMessage

/// <summary>
///   Handles [Ok] button.
/// </summary>
void FindDialogBox::onOk()
{
    switch (m_eDirection)
    {
    case Direction_Up:
        onFindPrevious();
        break;

    case Direction_Down:
        onFindNext();
        break;
    }
} // FindDialogBox::onOk

/// <summary>
///   Handles [Find Next] button.
/// </summary>
void FindDialogBox::onFindNext()
{
    DoFind(Direction_Down);
} // FindDialogBox::onFindNext

/// <summary>
///   Handles [Find Previous] button.
/// </summary>
void FindDialogBox::onFindPrevious()
{
    DoFind(Direction_Up);
} // FindDialogBox::onFindPrevious

/// <summary>
///   Handles [Replace All] button
/// </summary>
void FindDialogBox::onReplaceAll()
{
    doReplace(IDC_FIND_REPLACE_ALL);
} // FindDialogBox::onReplaceAll


/// <summary>
///   Handles [Replace] button
/// </summary>
void FindDialogBox::onReplaceOne()
{
    doReplace(IDC_FIND_REPLACE);
} // FindDialogBox::onReplaceOne

/// <summary>
///   Preparation for search. We extract search parameters from dialog box.
/// </summary>
/// <returns>Selection to start search.</returns>
Selection* FindDialogBox::prepareFind(SearchParameters* pSearch)
{
    ASSERT(NULL != pSearch);
    Selection* pSelection = getActiveSelection();
    if (NULL == pSelection)
    {
        // Active pane isn't editor.
        return NULL;
    }

    HWND hwndWhat = GetDlgItem(IDC_FIND_WHAT);

    pSearch->m_cwch = ::GetWindowTextLength(hwndWhat);
    if (0 == pSearch->m_cwch)
    {
        // Nothing to search
        return NULL;
    }

    ::GetWindowText(hwndWhat, pSearch->m_wsz, lengthof(pSearch->m_wsz));

    pSearch->m_rgf  = 0;

    if (! GetChecked(IDC_FIND_CASE)) 
    {
        for (const char16* pwsz = pSearch->m_wsz; 0 != *pwsz; pwsz++)
        {
            if (IsLowerCase(*pwsz))
            {
                pSearch->m_rgf |= SearchFlag_IgnoreCase;
            }
            else if (IsUpperCase(*pwsz))
            {
                pSearch->m_rgf &= ~SearchFlag_IgnoreCase;
                break;
            }
        } // for each char
    } // if "Match Case"

    if (GetChecked(IDC_FIND_PRESERVE))
    {
        char16 wszWith[100];
        ::GetWindowText(
            GetDlgItem(IDC_FIND_WITH),
            wszWith,
            lengthof(wszWith) );

        for (const char16* pwsz = wszWith; 0 != *pwsz; pwsz++)
        {
            if (IsLowerCase(*pwsz))
            {
                pSearch->m_rgf |= SearchFlag_CasePreserve;
            }
            else if (IsUpperCase(*pwsz))
            {
                pSearch->m_rgf &= ~SearchFlag_CasePreserve;
                break;
            }
        } // for each char
    } // if preserve

    if (GetChecked(IDC_FIND_REGEX))
    {
        pSearch->m_rgf |= SearchFlag_Regex;
    }

    if (GetChecked(IDC_FIND_WORD))
    {
        pSearch->m_rgf |= SearchFlag_MatchWord;
    }

    if (GetChecked(IDC_FIND_WHOLE_FILE))
    {
        pSearch->m_rgf |= SearchFlag_Whole;
    }

    return pSelection;
} // FindDialogBox::prepareFind

/// <summary>
///   Report no matched text.
/// </summary>
void FindDialogBox::reportNotFound()
{
    Application::instance()->GetActiveFrame()->ShowMessage(
        MessageLevel_Warning,
        IDS_NOT_FOUND );
} // FindDialogBox::reportNotFound

/// <summary>Updates find dialog box controls.</summary>
/// <param name="fActivate">True if dialog box is activated</param>
void FindDialogBox::updateUI(bool fActivate)
{
    SetCheckBox(IDC_FIND_DOWN, Direction_Down == m_eDirection);
    SetCheckBox(IDC_FIND_UP,   Direction_Up   == m_eDirection);

    Selection* pSelection = getActiveSelection();

    #if 0
        if (NULL == pSelection)
        {
            ::EnableWindow(*this, FALSE);
            return;
        }

        ::EnableWindow(*this, TRUE);
    #endif

    int cwch = ::GetWindowTextLength(GetDlgItem(IDC_FIND_WHAT));

    ::EnableWindow(GetDlgItem(IDC_FIND_NEXT),        cwch >= 1);
    ::EnableWindow(GetDlgItem(IDC_FIND_PREVIOUS),    cwch >= 1);
    ::EnableWindow(GetDlgItem(IDC_FIND_WITH),        cwch >= 1);
    ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE),     cwch >= 1);
    ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE_ALL), cwch >= 1);

    // If active selection covers mutliple lines, Search/Replace can be
    // limited in selection.
    bool fHasNewline = pSelection->FindFirstChar('\n') >= 0;
    ::EnableWindow(GetDlgItem(IDC_FIND_SELECTION),  fHasNewline);
    ::EnableWindow(GetDlgItem(IDC_FIND_WHOLE_FILE), fHasNewline);

    if (fActivate)
    {
        m_eReplaceIn = fHasNewline ? ReplaceIn_Selection : ReplaceIn_Whole;
    }

    SetCheckBox(IDC_FIND_SELECTION,  ReplaceIn_Selection == m_eReplaceIn);
    SetCheckBox(IDC_FIND_WHOLE_FILE, ReplaceIn_Whole     == m_eReplaceIn);
} // FindDialogBox::updateUI

#include "./cm_CmdProc.h"

namespace Command
{

// Global Find Dialog Box object
// FIXME 2007-07-17 yosi@msn.com We should initizlie controls from
// saved settings.
static FindDialogBox* s_pFindDialogBox;

/// <summary>
///   Activate find dialog box.
/// </summary>
DEFCOMMAND(FindCommand)
{
    ASSERT(NULL != pCtx);

    if (NULL == s_pFindDialogBox) 
    {
        s_pFindDialogBox = new FindDialogBox;
        s_pFindDialogBox->DoModeless();
    }

    ::ShowWindow(*s_pFindDialogBox, SW_SHOW);
    ::SetActiveWindow(*s_pFindDialogBox);
} // FindCommand

/// <summary>
///   Move selection to next match.
/// </summary>
DEFCOMMAND(FindNext)
{
    ASSERT(NULL != pCtx);

    if (NULL == s_pFindDialogBox) return;
    s_pFindDialogBox->DoFind(FindDialogBox::Direction_Down);
} // FindNext

/// <summary>
///   Move selection to previous match.
/// </summary>
DEFCOMMAND(FindPrevious)
{
    ASSERT(NULL != pCtx);

    if (NULL == s_pFindDialogBox) return;
    s_pFindDialogBox->DoFind(FindDialogBox::Direction_Up);
} // FindPrevious

} // Command
