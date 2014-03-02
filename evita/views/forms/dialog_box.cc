// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/dialog_box.h"

#include <utility>

#include "base/logging.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/public/api_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/views/forms/dialog_box_set.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;
extern HWND g_hwndActiveDialog;

namespace views {

namespace {

bool IsCheckBoxOrRadioButton(HWND hwnd){
  auto const style = ::GetWindowLong(hwnd, GWL_STYLE);
  auto const type = style & BS_TYPEMASK;
  return type == BS_AUTOCHECKBOX || type == BS_AUTORADIOBUTTON;
}

DialogBox* creating_dialog_box;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DialogBox
//
DialogBox::DialogBox(dom::Form* form)
    : dialog_box_id_(form->dialog_box_id()), dirty_(true), form_(form),
      hwnd_(nullptr) {
  DialogBoxSet::instance()->Register(this);
}

DialogBox::~DialogBox() {
  DialogBoxSet::instance()->Unregister(dialog_box_id_);
}

INT_PTR CALLBACK DialogBox::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam) {
  auto dialog_box = reinterpret_cast<DialogBox*>(
      ::GetWindowLongPtr(hwnd, DWLP_USER));

  if (!dialog_box) {
    dialog_box = creating_dialog_box;
    dialog_box->hwnd_ = hwnd;
    ::SetWindowLongPtr(hwnd, DWLP_USER,
                       reinterpret_cast<LONG_PTR>(dialog_box));
  }

  switch (uMsg) {
    case WM_ACTIVATE:
      if (WA_INACTIVE == wParam)
        g_hwndActiveDialog = nullptr;
      else
        g_hwndActiveDialog = dialog_box->hwnd_;
      break;

    case WM_COMMAND:
      dialog_box->onCommand(wParam, lParam);
      return 0;

    case WM_INITDIALOG:
      return dialog_box->onInitDialog();
  }

  return dialog_box->onMessage(uMsg, wParam, lParam);
}

void DialogBox::DidChangeFormContents() {
  dirty_ = true;
}

void DialogBox::DisableControl(int control_id) {
  EnableControl(control_id, false);
}

void DialogBox::DispatchFormEvent(const base::string16& type, int control_id,
                                  const base::string16& value) {
  domapi::FormEvent event {dialog_box_id_, control_id, type, value};
  Application::instance()->view_event_handler()->DispatchFormEvent(event);
}

void DialogBox::EnableControl(int control_id, bool enable) {
  auto const hwnd = GetDlgItem(control_id);
  DCHECK(hwnd);
  ::EnableWindow(hwnd, enable);
}

void DialogBox::FinishUpdateFromModel() {
  DCHECK(dirty_);
  dirty_ = false;
}

bool DialogBox::GetChecked(int item_id) const {
  return BST_CHECKED == ::SendMessage(GetDlgItem(item_id), BM_GETCHECK, 0, 0);
}

HWND DialogBox::GetDlgItem(int item_id) const { 
  return ::GetDlgItem(hwnd_, item_id);
}

base::string16 DialogBox::GetDlgItemText(int item_id) const {
  auto const hwnd = GetDlgItem(item_id);
  auto const length = ::GetWindowTextLength(hwnd);
  if (!length)
    return base::string16();
  // +1 for terminating zero.
  base::string16 text(static_cast<size_t>(length + 1), '?');
  auto const length2 = ::GetWindowTextW(hwnd, &text[0],
                                        static_cast<int>(text.length()));
  DCHECK_EQ(length, length2);
  text.resize(static_cast<size_t>(length2));
  return std::move(text);
}

void DialogBox::onOk() {
  ::EndDialog(hwnd_, IDOK);
}

void DialogBox::onCancel() {
  ::EndDialog(hwnd_, IDCANCEL);
}

bool DialogBox::onCommand(WPARAM wParam, LPARAM lParam) {
  auto const hwnd = reinterpret_cast<HWND>(lParam);
  base::string16 class_name(100, '?');
  auto const class_name_length = ::GetClassName(hwnd, &class_name[0],
      static_cast<int>(class_name.length()));
  class_name.resize(static_cast<size_t>(class_name_length));
  auto const code = HIWORD(wParam);
  auto const control_id = LOWORD(wParam);

  if (class_name == L"Button") {
    if (IsCheckBoxOrRadioButton(hwnd)) {
      if (code == BN_CLICKED) {
        DispatchFormEvent(L"change", control_id,
                          Button_GetCheck(hwnd) == BST_CHECKED ? L"1" :
                          base::string16());
      }
      return false;
    }

    if (code == BN_CLICKED) {
      DispatchFormEvent(L"click", control_id, base::string16());
      return false;
    }
    return false;
  }

  if (class_name == L"ComboBox") {
    switch (code) {
      // Note: It seems CBN_EDITCHANGE isn't sent.
      case CBN_EDITCHANGE:
      case CBN_EDITUPDATE:
      case CBN_SELCHANGE:
      case CBN_SELENDCANCEL:
        DispatchFormEvent(L"change", control_id, GetDlgItemText(control_id));
        return false;
    }
  }

  return false;
}

bool DialogBox::OnIdle(int) {
  return false;
}

INT_PTR DialogBox::onMessage(UINT, WPARAM, LPARAM) {
  return 0;
}

void DialogBox::Realize() {
  creating_dialog_box = this;
  hwnd_ = ::CreateDialogParam(
      g_hInstance,
      MAKEINTRESOURCE(GetTemplate()),
      nullptr,
      DialogProc,
      reinterpret_cast<LPARAM>(this));
}

int DialogBox::SetCheckBox(int item_id, bool checked) {
  return static_cast<int>(::SendMessage(GetDlgItem(item_id), BM_SETCHECK,
      static_cast<WPARAM>(checked ? BST_CHECKED : BST_UNCHECKED), 0));
}

void DialogBox::Show() {
  ::ShowWindow(*this, SW_SHOW);
  ::SetActiveWindow(*this);
}

void DialogBox::UpdateCheckboxFromModel(int control_id) {
  UI_ASSERT_DOM_LOCKED();
  auto const control = form_->control(control_id);
  if (!control) {
    DVLOG(0) << "No such control " << control_id;
    return;
  }
  auto const checkbox_control = control->as<dom::CheckboxControl>();
  if (!checkbox_control) {
    DVLOG(0) << "Control " << control_id << " isn't checkbox control.";
    return;
  }
  auto const checked = checkbox_control->checked();
  ::SendMessage(GetDlgItem(control_id), BM_SETCHECK,
      static_cast<WPARAM>(checked ? BST_CHECKED : BST_UNCHECKED), 0);
}

void DialogBox::UpdateTextFromModel(int control_id) {
  UI_ASSERT_DOM_LOCKED();
  auto const control = form_->control(control_id);
  if (!control) {
    DVLOG(0) << "No such control " << control_id;
    return;
  }
  auto const text_control = control->as<dom::TextFieldControl>();
  if (!text_control) {
    DVLOG(0) << "Control " << control_id << " isn't text field control.";
    return;
  }
  ::SetWindowTextW(GetDlgItem(control_id), text_control->value().c_str());
}

void DialogBox::UpdateRadioButtonFromModel(int control_id) {
  UI_ASSERT_DOM_LOCKED();
  auto const control = form_->control(control_id);
  if (!control) {
    DVLOG(0) << "No such control " << control_id;
    return;
  }
  auto const radio_button_control = control->as<dom::RadioButtonControl>();
  if (!radio_button_control) {
    DVLOG(0) << "Control " << control_id << " isn't radio_button control.";
    return;
  }
  auto const checked = radio_button_control->checked();
  ::SendMessage(GetDlgItem(control_id), BM_SETCHECK,
      static_cast<WPARAM>(checked ? BST_CHECKED : BST_UNCHECKED), 0);
}

}  // namespace views
