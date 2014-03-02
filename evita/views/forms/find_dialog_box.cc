// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/find_dialog_box.h"

#include "base/logging.h"
#include "evita/resource.h"
#include "evita/text/range.h"
#include "evita/views/frame_list.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

namespace views {

#define BEGIN_COMMAND_MAP switch (wParam) {
#define END_COMMAND_MAP } return DialogBox::onCommand(wParam, lParam);

#define ON_COMMAND(mp_ctrl, mp_notify, mp_method, ...) \
  case MAKEWPARAM(mp_ctrl, mp_notify): { \
    mp_method(__VA_ARGS__); \
    return true; \
  }

namespace {

Selection* GetActiveSelection() {
  auto const frame = views::FrameList::instance()->active_frame();
  if (!frame)
    return nullptr;
  auto const edit_pane = frame->GetActivePane()->as<EditPane>();
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

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
FindDialogBox::FindDialogBox(DialogBoxId dialog_box_id)
    : DialogBox(dialog_box_id),
      direction_(kDirectionDown),
      replace_in_(kReplaceInWhole) {
}

FindDialogBox::~FindDialogBox() {
}

/// <summary>Updates find dialog box controls.</summary>
/// <param name="fActivate">True if dialog box is activated</param>
void FindDialogBox::UpdateUI(bool fActivate) {
  SetCheckBox(IDC_FIND_DOWN, kDirectionDown == direction_);
  SetCheckBox(IDC_FIND_UP, kDirectionUp == direction_);

  auto const cwch = ::GetWindowTextLength(GetDlgItem(IDC_FIND_WHAT));

  ::EnableWindow(GetDlgItem(IDC_FIND_NEXT), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_PREVIOUS), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_WITH), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE_ALL), cwch >= 1);

 // If active selection covers mutliple lines, Search/Replace can be
 // limited in selection.
  if (auto const selection = GetActiveSelection()) {
    auto const fHasNewline = selection->range()->FindFirstChar('\n') >= 0;
    ::EnableWindow(GetDlgItem(IDC_FIND_SELECTION), fHasNewline);
    ::EnableWindow(GetDlgItem(IDC_FIND_WHOLE_FILE), fHasNewline);

    if (fActivate) {
      replace_in_ = fHasNewline ? kReplaceInSelection : kReplaceInWhole;
    }
    SetCheckBox(IDC_FIND_SELECTION, kReplaceInSelection == replace_in_);
    SetCheckBox(IDC_FIND_WHOLE_FILE, kReplaceInWhole == replace_in_);
  }
}

// views::DialogBox
int FindDialogBox::GetTemplate() const {
  return IDD_FIND;
}

void FindDialogBox::onCancel() {
  ::ShowWindow(*this, SW_HIDE);
  ::SetActiveWindow(*views::FrameList::instance()->active_frame());
}

bool FindDialogBox::onCommand(WPARAM wParam, LPARAM lParam) {
  BEGIN_COMMAND_MAP
    ON_COMMAND(IDOK, BN_CLICKED, onOk) // [Enter] key
    ON_COMMAND(IDCANCEL, BN_CLICKED, onCancel) // [Esc] key
    ON_COMMAND(IDC_FIND_WHAT, CBN_EDITCHANGE, UpdateUI)
    ON_COMMAND(IDC_FIND_WITH, CBN_EDITCHANGE, UpdateUI)
  END_COMMAND_MAP
}

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

  // Returns false not to set focus to dialog.
  return false;
}

INT_PTR FindDialogBox::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_ACTIVATE:
      if (WA_INACTIVE == wParam)
        return FALSE;

      UpdateUI(true);
      ::SetFocus(GetDlgItem(IDC_FIND_WHAT));
      return TRUE;

    case WM_WINDOWPOSCHANGED: {
      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
      if (wp->flags & SWP_HIDEWINDOW) {
        views::FrameList::instance()->active_frame()->GetActivePane()->
            RequestFocus();
        return TRUE;
      }
      break;
    }
  }
  return DialogBox::onMessage(uMsg, wParam, lParam);
}

void FindDialogBox::onOk() {
  // Simulate CBN_KILLFOCUS
  onCommand(MAKEWPARAM(IDC_FIND_WHAT, CBN_KILLFOCUS),
            reinterpret_cast<LPARAM>(GetDlgItem(IDC_FIND_WHAT)));
  onCommand(MAKEWPARAM(IDC_FIND_WITH, CBN_KILLFOCUS),
            reinterpret_cast<LPARAM>(GetDlgItem(IDC_FIND_WITH)));
  auto const control_id = direction_ == kDirectionUp ?
      IDC_FIND_PREVIOUS : IDC_FIND_NEXT;
  onCommand(MAKEWPARAM(control_id, BN_CLICKED),
            reinterpret_cast<LPARAM>(GetDlgItem(control_id)));
}

}  // namespace views
