// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/dialog_box.h"

#include <ostream>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/views/forms/dialog_box_set.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;
extern HWND g_hwndActiveDialog;

namespace views {

namespace {

enum class ControlType {
  Unknown,
  Button,
  Checkbox,
  ComboBox,
  RadioButton,
  TextField,
};

std::ostream& operator<<(std::ostream& ostream, ControlType type) {
  switch (type) {
    case ControlType::Button:
      return ostream << "Button";
    case ControlType::Checkbox:
      return ostream << "Checkbox";
    case ControlType::ComboBox:
      return ostream << "ComboBox";
    case ControlType::RadioButton:
      return ostream << "RadioButton";
    case ControlType::TextField:
      return ostream << "TextField";
    case ControlType::Unknown:
      return ostream << "Unknown";
  }
  return ostream << "ControlType(" << static_cast<int>(type) << ")";
}

ControlType GetControlType(HWND hwnd) {
  base::string16 class_name(100, '?');
  auto const class_name_length = ::GetClassName(hwnd, &class_name[0],
      static_cast<int>(class_name.length()));
  class_name.resize(static_cast<size_t>(class_name_length));
  if (class_name == L"Button") {
    auto const style = ::GetWindowLong(hwnd, GWL_STYLE);
    auto const type = style & BS_TYPEMASK;
    if (type == BS_CHECKBOX || type == BS_AUTOCHECKBOX)
      return ControlType::Checkbox;
    if (type == BS_RADIOBUTTON || type == BS_AUTORADIOBUTTON)
      return ControlType::RadioButton;
    if (type == BS_3STATE || type == BS_AUTO3STATE)
      return ControlType::Checkbox;
    return ControlType::Button;
  }
  if (class_name == L"ComboBox")
    return ControlType::ComboBox;
  if (class_name == L"Edit")
    return ControlType::TextField;
  return ControlType::Unknown;
}

base::string16 GetWindowText(HWND hwnd) {
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

std::vector<std::pair<HWND, int>> ListControls(HWND hwnd) {
  class ChildWindowEnumerator {
    private: std::vector<std::pair<HWND, int>> children_;
    private: HWND hwnd_;

    public: ChildWindowEnumerator(HWND hwnd) : hwnd_(hwnd) {
    }

    public: ~ChildWindowEnumerator() = default;

    private: static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
      auto const self = reinterpret_cast<ChildWindowEnumerator*>(lParam);
      if (::GetParent(hwnd) != self->hwnd_)
        return TRUE;
      auto const control_id = static_cast<int>(::GetWindowLong(hwnd, GWL_ID));
      if (control_id != -1)
        self->children_.push_back(std::pair<HWND, int>(hwnd, control_id));
      return TRUE;
    }

    public: std::vector<std::pair<HWND, int>> Run() {
      ::EnumChildWindows(hwnd_, EnumChildProc, reinterpret_cast<LPARAM>(this));
      return std::move(children_);
    }
  };
  ChildWindowEnumerator enumerator(hwnd);
  return enumerator.Run();
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

void DialogBox::DispatchFormEvent(domapi::EventType event_type, int control_id,
                                  const base::string16& value) {
  domapi::FormEvent event;
  event.event_type = event_type;
  event.target_id = dialog_box_id_;
  event.control_id = control_id;
  event.data = value;
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
  return std::move(GetWindowText(GetDlgItem(item_id)));
}

void DialogBox::onOk() {
  ::EndDialog(hwnd_, IDOK);
}

void DialogBox::onCancel() {
  ::EndDialog(hwnd_, IDCANCEL);
}

bool DialogBox::onCommand(WPARAM wParam, LPARAM lParam) {
  auto const hwnd = reinterpret_cast<HWND>(lParam);
  auto const code = HIWORD(wParam);
  auto const control_id = LOWORD(wParam);
  auto const control_type = GetControlType(hwnd);

  switch (control_type) {
    case ControlType::Button:
      if (code == BN_CLICKED)
        DispatchFormEvent(domapi::EventType::FormClick, control_id,
                          base::string16());
      break;
    case ControlType::Checkbox:
    case ControlType::RadioButton:
      if (code == BN_CLICKED) {
        DispatchFormEvent(domapi::EventType::FormChange, control_id,
                          Button_GetCheck(hwnd) == BST_CHECKED ? L"1" :
                          base::string16());
      }
      break;
    case ControlType::ComboBox:
      if (code == CBN_EDITCHANGE)
          DispatchFormEvent(domapi::EventType::FormChange, control_id,
                            GetDlgItemText(control_id));
      break;
  }
  return false;
}

bool DialogBox::OnIdle(int) {
  if (!realized() || !dirty_)
    return false;

  const auto controls = ListControls(hwnd_);

  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked()) {
    // Note: We keep focus control enabled not to interrupt user input.
    auto const focus_hwnd = ::GetFocus();
    auto const focus_parent_hwnd = ::GetParent(focus_hwnd);
    for (auto pair : controls) {
      if (pair.first != focus_hwnd &&  pair.first != focus_parent_hwnd)
        ::EnableWindow(pair.first, false);
    }
    dirty_ = true;
    return true;
  }

  for (auto pair : controls) {
    UpdateControlFromModel(pair.first, pair.second);
  }
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

void DialogBox::UpdateControlFromModel(HWND hwnd, int control_id) {
  UI_ASSERT_DOM_LOCKED();
  auto const control = form_->control(control_id);
  if (!control) {
    DVLOG(1) << "No such control " << control_id;
    return;
  }
  auto const control_type = GetControlType(hwnd);
  switch (control_type) {
    case ControlType::Button:
      // TODO(yosi): Should we support button label?
      break;
    case ControlType::Checkbox: {
      auto const checkbox_control = control->as<dom::CheckboxControl>();
      if (!checkbox_control) {
        DVLOG(0) << "Control " << control_id << " isn't checkbox control.";
        break;
      }
      auto const checked = checkbox_control->checked();
      Button_SetCheck(hwnd, checked ? BST_CHECKED : BST_UNCHECKED);
      break;
    }
    case ControlType::ComboBox:
    case ControlType::TextField:
      if (auto const text_control = control->as<dom::TextFieldControl>()) {
        if (GetWindowText(hwnd) != text_control->value())
          ::SetWindowTextW(hwnd, text_control->value().c_str());
      }
      break;
    case ControlType::RadioButton: {
      auto const radio_button_control = control->as<dom::RadioButtonControl>();
      if (!radio_button_control) {
        DVLOG(0) << "Control " << control_id << " isn't radio_button control.";
        break;
      }
      auto const checked = radio_button_control->checked();
      Button_SetCheck(hwnd, checked ? BST_CHECKED : BST_UNCHECKED);
      break;
    }
    default:
      DVLOG(0) << "Ignore unknown control type " << control_type;
      break;
  }
  ::EnableWindow(hwnd, !control->disabled());
}

}  // namespace views
