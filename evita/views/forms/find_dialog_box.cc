// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/find_dialog_box.h"

#include "base/logging.h"
#include "evita/editor/dom_lock.h"
#include "evita/views/frame_list.h"
#include "evita/vi_Frame.h"
#include "evita/resource.h"

namespace views {

#define BEGIN_COMMAND_MAP switch (wParam) {
#define END_COMMAND_MAP } return DialogBox::onCommand(wParam, lParam);

#define ON_COMMAND(mp_ctrl, mp_notify, mp_method, ...) \
  case MAKEWPARAM(mp_ctrl, mp_notify): { \
    mp_method(__VA_ARGS__); \
    return true; \
  }

FindDialogBox::FindDialogBox(dom::Form* form)
    : DialogBox(form),
      direction_(kDirectionDown),
      replace_in_(kReplaceInWhole) {
}

FindDialogBox::~FindDialogBox() {
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
  END_COMMAND_MAP
}

bool FindDialogBox::onInitDialog() {
  auto const dwExStyle = ::GetWindowLong(*this, GWL_EXSTYLE) | WS_EX_LAYERED;
  ::SetWindowLong(*this, GWL_EXSTYLE, dwExStyle);

 // FIXME 2007-08-20 yosi@msn.com We should get default value of
 // find dialog transparency.
  ::SetLayeredWindowAttributes(*this, RGB(0, 0, 0), 80 * 255 / 100, LWA_ALPHA);

  // Returns false not to set focus to dialog.
  return false;
}

bool FindDialogBox::OnIdle(int) {
  if (!realized())
    return false;

  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked()) {
    // Note: We keep text fields enabled not to interrupt user input.

    // Use
    DisableControl(IDC_FIND_EXACT);
    DisableControl(IDC_FIND_WORD);
    DisableControl(IDC_FIND_REGEX);

    // Checkboxes
    DisableControl(IDC_FIND_CASE);
    DisableControl(IDC_FIND_ALLDOCS);
    DisableControl(IDC_FIND_PRESERVE);

    // Direction
    DisableControl(IDC_FIND_DIRECTION);
    DisableControl(IDC_FIND_UP);
    DisableControl(IDC_FIND_DOWN);

    // Replace In
    DisableControl(IDC_FIND_REPLACE_IN);
    DisableControl(IDC_FIND_SELECTION);
    DisableControl(IDC_FIND_WHOLE_FILE);

    // buttons
    DisableControl(IDC_FIND_NEXT);
    DisableControl(IDC_FIND_PREVIOUS);
    DisableControl(IDC_FIND_REPLACE);
    DisableControl(IDC_FIND_REPLACE_ALL);
    return true;
  }

  if (dirty()) {
    UpdateTextFromModel(IDC_FIND_WHAT);
    UpdateTextFromModel(IDC_FIND_WITH);

    UpdateCheckboxFromModel(IDC_FIND_CASE);
    UpdateCheckboxFromModel(IDC_FIND_PRESERVE);
    UpdateCheckboxFromModel(IDC_FIND_ALLDOCS);

    UpdateRadioButtonFromModel(IDC_FIND_EXACT);
    UpdateRadioButtonFromModel(IDC_FIND_WORD);
    UpdateRadioButtonFromModel(IDC_FIND_REGEX);

    UpdateRadioButtonFromModel(IDC_FIND_UP);
    UpdateRadioButtonFromModel(IDC_FIND_DOWN);

    UpdateRadioButtonFromModel(IDC_FIND_SELECTION);
    UpdateRadioButtonFromModel(IDC_FIND_WHOLE_FILE);

    FinishUpdateFromModel();
  }

  EnableControl(IDC_FIND_WHAT, true);

  auto const can_find = !GetDlgItemText(IDC_FIND_WHAT).empty();
  EnableControl(IDC_FIND_NEXT, can_find);
  EnableControl(IDC_FIND_PREVIOUS, can_find);
  EnableControl(IDC_FIND_WORD, can_find);
  EnableControl(IDC_FIND_CASE, can_find);
  EnableControl(IDC_FIND_REGEX, can_find);
  EnableControl(IDC_FIND_ALLDOCS, can_find);
  EnableControl(IDC_FIND_DIRECTION, can_find);
  EnableControl(IDC_FIND_UP, can_find);
  EnableControl(IDC_FIND_DOWN, can_find);
  EnableControl(IDC_FIND_EXACT, can_find);
  EnableControl(IDC_FIND_WITH, can_find);

  auto const can_replace = !GetDlgItemText(IDC_FIND_WITH).empty();
  EnableControl(IDC_FIND_REPLACE, can_replace);
  EnableControl(IDC_FIND_REPLACE_ALL, can_replace);
  EnableControl(IDC_FIND_PRESERVE, can_replace);
  EnableControl(IDC_FIND_REPLACE_IN, can_replace);
  EnableControl(IDC_FIND_SELECTION, can_replace);
  EnableControl(IDC_FIND_WHOLE_FILE, can_replace);

  return false;
}

INT_PTR FindDialogBox::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_ACTIVATE:
      if (WA_INACTIVE == wParam)
        return FALSE;
      ::SetFocus(GetDlgItem(IDC_FIND_WHAT));
      return TRUE;

    case WM_WINDOWPOSCHANGED: {
      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
      if (wp->flags & SWP_HIDEWINDOW) {
        views::FrameList::instance()->active_frame()->RequestFocus();
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
