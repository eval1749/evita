#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/vi_FindDialogBox.h"

#include <utility>

#include "base/logging.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/RegexMatch.h"
#include "evita/text/search_and_replace_model.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

#define BEGIN_COMMAND_MAP switch (wParam) {
#define END_COMMAND_MAP } return false;

#define ON_COMMAND(mp_ctrl, mp_notify, mp_method) \
  case MAKEWPARAM(mp_ctrl, mp_notify): { \
    mp_method(); \
    return true; \
  }

namespace {

// Replaces region with specified case.
void CaseReplace(text::Range* range, StringCase string_case) {
  switch (string_case) {
    case StringCase_Capitalized:
      range->Capitalize();
      break;

    case StringCase_CapitalizedAll:
      range->CapitalizeAll();
      break;

    case StringCase_Lower:
      range->Downcase();
      break;

    case StringCase_Mixed:
    case StringCase_None:
      // Nothing to do
      break;

    case StringCase_Upper:
      range->Upcase();
      break;
  }
}

Selection* GetActiveSelection() {
  auto const edit_pane = Application::instance()->GetActiveFrame()->
    GetActivePane()->DynamicCast<EditPane>();

  if (!edit_pane)
    return nullptr;

  auto const window = edit_pane->GetActiveWindow();
  if (!window)
    return nullptr;

  if (auto const text_edit_window = window->as<TextEditWindow>())
    return text_edit_window->GetSelection();

  return nullptr;
}
}   // namespace

FindDialogBox::FindDialogBox()
    : direction_(kDirectionDown),
      replace_in_(kReplaceInWhole) {
}

FindDialogBox::~FindDialogBox() {
}

void FindDialogBox::ClearMessage() {
  Application::instance()->GetActiveFrame()->ShowMessage(MessageLevel_Warning);
}

void FindDialogBox::DoFind(Direction eDirection) {
  ClearMessage();

  direction_ = eDirection;

  SearchParameters search;
  auto const selection = PrepareFind(&search);
  if (!selection) {
    // We may not have search text.
    return;
  }

  if (kDirectionUp == eDirection)
    search.m_rgf |= SearchFlag_Backward;

  auto const buffer = selection->GetBuffer();
  auto start_position = selection->GetStart();
  auto end_position = selection->GetEnd();
  if (search.IsWhole()){
    if (search.IsBackward()) {
      start_position = 0;
      end_position = selection->GetStart();
    } else {
      start_position = selection->GetEnd();
      end_position = buffer->GetEnd();
    }
  }

  RegexMatcher matcher(&search, selection->GetBuffer(), start_position,
                       end_position);

  {
    int nChar;
    if (int nError = matcher.GetError(&nChar)) {
      Application::instance()->GetActiveFrame()->ShowMessage(
          MessageLevel_Warning,
          IDS_BAD_REGEX,
          nChar,
          nError);
      return;
    }
  }

  if (!FindFirst(&matcher)) {
    ReportNotFound();
    return;
  }

  selection->SetRange(matcher.GetMatched(0));
  selection->SetStartIsActive(search.IsBackward());
  selection->GetWindow()->MakeSelectionVisible();

  UpdateUI();
}

void FindDialogBox::DoReplace(ReplaceMode replace_mode) {
  ClearMessage();

  SearchParameters search;
  auto const selection = PrepareFind(&search);
  if (!selection) {
    // We may not have search text.
    return;
  }

  auto const buffer = selection->GetBuffer();
  auto start_position = selection->GetStart();
  auto end_position = selection->GetEnd();
  if (search.IsWhole()) {
    start_position = buffer->GetStart();
    end_position = buffer->GetEnd();
  }

  RegexMatcher matcher(&search, selection->GetBuffer(), start_position,
                       end_position);

  if (!FindFirst(&matcher)) {
    ReportNotFound();
    return;
  }

  auto const wszWith = GetDlgItemText(IDC_FIND_WITH);
  auto const cwchWith = static_cast<int>(wszWith.length());

  bool is_replace_with_meta = search.m_rgf & SearchFlag_Regex;

  Count num_replaced = 0;

  if (replace_mode == kReplaceOne) {
    auto range = matcher.GetMatched(0);
    if (selection->GetStart() == range->GetStart() &&
      selection->GetEnd() == range->GetEnd()) {
      if (search.IsCasePreserve()) {
        text::UndoBlock oUndo(buffer, L"Edit.Replace");
        StringCase eCase = selection->AnalyzeCase();
        matcher.Replace(wszWith.data(), cwchWith, is_replace_with_meta);
        CaseReplace(selection, eCase);
      } else {
        matcher.Replace(wszWith.data(), cwchWith, is_replace_with_meta);
      }
      ++num_replaced;
    }

    // Just select matched string or replaced string.
    selection->SetRange(range);
    selection->SetStartIsActive(false);
  } else {
    // Replace multiple matched strings
    text::UndoBlock oUndo(buffer, L"Edit.Replace");

    text::Range oRange(buffer, start_position, end_position);

    do {
      auto const range = matcher.GetMatched(0);
      DCHECK(range);

      bool fEmptyMatch = range->GetStart() == range->GetEnd();

      if (search.IsCasePreserve()) {
        StringCase eCase = range->AnalyzeCase();
        matcher.Replace(wszWith.data(), cwchWith, is_replace_with_meta);
        CaseReplace(range, eCase);
      } else {
        matcher.Replace(wszWith.data(), cwchWith, is_replace_with_meta);
      }

      ++num_replaced;

      if (fEmptyMatch) {
        auto const position = range->GetEnd();
        if (buffer->GetEnd() == position) {
          // We reach at end of buffer.
          break;
        }
        range->SetEnd(position + 1);
      }

      // FIXME 2008-07-09 yosi@msn.com We should allow interrupt
      // replacing.
      range->Collapse(Collapse_End);
    } while (matcher.NextMatch());
  }

  Application::instance()->GetActiveFrame()->ShowMessage(
      MessageLevel_Information,
      IDS_REPLACED,
      num_replaced);

  selection->GetWindow()->MakeSelectionVisible();
}

/// <summary>
///   Find the first match by executing matcher.
/// </summary>
/// <param name="pMatcher">
///   A matcher initialized from Find Dialog input elements.
/// </param>
bool FindDialogBox::FindFirst(RegexMatcher* pMatcher) {
  if (pMatcher->FirstMatch())
    return true;

  if (pMatcher->WrapMatch()) {
    Application::instance()->ShowMessage(MessageLevel_Warning, IDS_PASSED_END);
    return true;
  }

  return false;
}

/// <summary>
///   Handles [Cancel] button.
/// </summary>
void FindDialogBox::onCancel() {
  ClearMessage();
  ::ShowWindow(*this, SW_HIDE);
  ::SetActiveWindow(*Application::instance()->GetActiveFrame());
}

/// <summary>
///   Dispatch WM_COMMAND to controls.
/// </summary>
/// <param name="wParam">
///   wParam of window message
/// </param>
/// <param name="lParam">
///   lParam of window message
/// </param>
bool FindDialogBox::onCommand(WPARAM wParam, LPARAM) {
  DVLOG(1) << "FindDialogBox::onCommand ctrlid=" << LOWORD(wParam) <<
      " notify=" << std::hex << HIWORD(wParam);

  BEGIN_COMMAND_MAP
    ON_COMMAND(IDOK, BN_CLICKED, onOk) // [Entery] key
    ON_COMMAND(IDCANCEL, BN_CLICKED, onCancel) // [Esc] key

    ON_COMMAND(IDC_FIND_NEXT, BN_CLICKED, onFindNext)
    ON_COMMAND(IDC_FIND_PREVIOUS, BN_CLICKED, onFindPrevious)
    ON_COMMAND(IDC_FIND_REPLACE, BN_CLICKED, onReplaceOne)
    ON_COMMAND(IDC_FIND_REPLACE_ALL, BN_CLICKED, onReplaceAll)
    ON_COMMAND(IDC_FIND_WHAT, CBN_EDITCHANGE, UpdateUI)
    ON_COMMAND(IDC_FIND_WITH, CBN_EDITCHANGE, UpdateUI)
  END_COMMAND_MAP
}

/// <summary>
///   Initialize Find dialog box. This method is called at WM_INITDIALOG.
/// </summary>
bool FindDialogBox::onInitDialog() {
  auto const dwExStyle = ::GetWindowLong(*this, GWL_EXSTYLE) |
      WS_EX_LAYERED;
  ::SetWindowLong(*this, GWL_EXSTYLE, dwExStyle);

 // FIXME 2007-08-20 yosi@msn.com We should get default value of
 // find dialog transparency.
  ::SetLayeredWindowAttributes(
      *this,
      RGB(0, 0, 0),
      80 * 255 / 100,
      LWA_ALPHA);

  SetCheckBox(IDC_FIND_EXACT, true);

 // FIXME 2007-08-20 yosi@msn.com We should get default value of
 // case-preserving replace.
  SetCheckBox(IDC_FIND_PRESERVE, true);

  UpdateUI(true);

  return true;
}

/// <summary>
///   Dispatch windows message.
/// </summary>
INT_PTR FindDialogBox::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
 //DEBUG_PRINTF("%p uMsg=0x%04x\n", this, uMsg);
  switch (uMsg) {
    case WM_ACTIVATE:
      DVLOG(1) << "WM_ACTIVATE wParam=" << wParam;

      if (WA_INACTIVE == wParam) {
        //::SetActiveWindow(*Application::instance()->GetActiveFrame());
        //::SetFocus(*Application::instance()->GetActiveFrame());
        return FALSE;
      }

      UpdateUI(true);
      ::SetFocus(GetDlgItem(IDC_FIND_WHAT));
      return TRUE;

    case WM_WINDOWPOSCHANGED: {
      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
      if (wp->flags & SWP_HIDEWINDOW) {
        DVLOG(0) << "WM_WINDOWPOSCHANGED: SWP_HIDEWINDOW";
        //::SetActiveWindow(*Application::instance()->GetActiveFrame());
        Application::instance()->GetActiveFrame()->GetActivePane()->SetFocus();
        return TRUE;
      }
      break;
    }
  }
  return DialogBox::onMessage(uMsg, wParam, lParam);
}

void FindDialogBox::onOk() {
  switch (direction_) {
    case kDirectionUp:
      onFindPrevious();
      break;
    case kDirectionDown:
      onFindNext();
      break;
  }
}

void FindDialogBox::onFindNext() {
  DoFind(kDirectionDown);
}

void FindDialogBox::onFindPrevious() {
  DoFind(kDirectionUp);
}

void FindDialogBox::onReplaceAll() {
  DoReplace(kReplaceAll);
}

void FindDialogBox::onReplaceOne() {
  DoReplace(kReplaceOne);
}

/// <summary>
///   Preparation for search. We extract search parameters from dialog box.
/// </summary>
/// <returns>Selection to start search.</returns>
Selection* FindDialogBox::PrepareFind(SearchParameters* search) {
  DCHECK(search);
  auto const selection = GetActiveSelection();
  if (!selection) {
    // Active pane isn't editor.
    return nullptr;
  }

  search->search_text_ = std::move(GetDlgItemText(IDC_FIND_WHAT));
  if (search->search_text_.empty())
    return nullptr;
  search->m_rgf = 0;

  if (!GetChecked(IDC_FIND_CASE))  {
    for (auto const ch : search->search_text_) {
      if (IsLowerCase(ch)) {
        search->m_rgf |= SearchFlag_IgnoreCase;
      } else if (IsUpperCase(ch)) {
        search->m_rgf &= ~SearchFlag_IgnoreCase;
        break;
      }
    }
  }

  if (GetChecked(IDC_FIND_PRESERVE)) {
    auto const wszWith = GetDlgItemText(IDC_FIND_WITH);
    for (auto ch : wszWith) {
      if (IsLowerCase(ch)) {
        search->m_rgf |= SearchFlag_CasePreserve;
      } else if (IsUpperCase(ch)) {
        search->m_rgf &= ~SearchFlag_CasePreserve;
        break;
      }
    }
  }

  if (GetChecked(IDC_FIND_REGEX))
    search->m_rgf |= SearchFlag_Regex;

  if (GetChecked(IDC_FIND_WORD))
    search->m_rgf |= SearchFlag_MatchWord;

  if (GetChecked(IDC_FIND_WHOLE_FILE))
    search->m_rgf |= SearchFlag_Whole;

  return selection;
}

void FindDialogBox::ReportNotFound() {
  Application::instance()->GetActiveFrame()->ShowMessage(
      MessageLevel_Warning,
      IDS_NOT_FOUND);
}

/// <summary>Updates find dialog box controls.</summary>
/// <param name="fActivate">True if dialog box is activated</param>
void FindDialogBox::UpdateUI(bool fActivate) {
  SetCheckBox(IDC_FIND_DOWN, kDirectionDown == direction_);
  SetCheckBox(IDC_FIND_UP, kDirectionUp == direction_);

  auto const selection = GetActiveSelection();
  auto const cwch = ::GetWindowTextLength(GetDlgItem(IDC_FIND_WHAT));

  ::EnableWindow(GetDlgItem(IDC_FIND_NEXT), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_PREVIOUS), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_WITH), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE_ALL), cwch >= 1);

 // If active selection covers mutliple lines, Search/Replace can be
 // limited in selection.
  auto const fHasNewline = selection->FindFirstChar('\n') >= 0;
  ::EnableWindow(GetDlgItem(IDC_FIND_SELECTION), fHasNewline);
  ::EnableWindow(GetDlgItem(IDC_FIND_WHOLE_FILE), fHasNewline);

  if (fActivate)
    replace_in_ = fHasNewline ? kReplaceInSelection : kReplaceInWhole;

  SetCheckBox(IDC_FIND_SELECTION, kReplaceInSelection == replace_in_);
  SetCheckBox(IDC_FIND_WHOLE_FILE, kReplaceInWhole == replace_in_);
}

#include "./cm_CmdProc.h"

namespace Command {

// Global Find Dialog Box object
// FIXME 2007-07-17 yosi@msn.com We should initialize controls from
// saved settings.
static FindDialogBox* s_pFindDialogBox;

DEFCOMMAND(FindCommand) {
  DCHECK(pCtx);
  if (!s_pFindDialogBox) {
    s_pFindDialogBox = new FindDialogBox;
    s_pFindDialogBox->DoModeless();
  }

  ::ShowWindow(*s_pFindDialogBox, SW_SHOW);
  ::SetActiveWindow(*s_pFindDialogBox);
}

DEFCOMMAND(FindNext) {
  DCHECK(pCtx);
  if (!s_pFindDialogBox)
    return;
  s_pFindDialogBox->DoFind(FindDialogBox::kDirectionDown);
}

DEFCOMMAND(FindPrevious) {
  DCHECK(pCtx);
  if (!s_pFindDialogBox)
    return;
  s_pFindDialogBox->DoFind(FindDialogBox::kDirectionUp);
}

}  // namespace Command
