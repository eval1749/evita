// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_dialog_box_h)
#define INCLUDE_evita_views_forms_dialog_box_h

#include <memory>

#include "base/strings/string16.h"
#include "evita/dom/public/dialog_box_id.h"
#include "evita/gc/member.h"

typedef domapi::DialogBoxId DialogBoxId;
const DialogBoxId kInvalidDialogBoxId = domapi::kInvalidDialogBoxId;

namespace dom {
class Form;
}

#undef DialogBox

namespace views {

//////////////////////////////////////////////////////////////////////
//
// DialogBox
//
class DialogBox {
  private: DialogBoxId dialog_box_id_;
  private: bool dirty_;
  private: gc::Member<dom::Form> form_;
  private: HWND hwnd_;

  protected: DialogBox(dom::Form* form);
  public: virtual ~DialogBox();

  protected: operator HWND() const { return hwnd_; }
  public: DialogBoxId dialog_box_id() const { return dialog_box_id_; }
  protected: const dom::Form* form() const { return form_.get(); }
  protected: bool realized() const { return hwnd_; }

  private: static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message,
                                              WPARAM wParam, LPARAM lParam);
  public: void DidChangeFormContents();
  protected: void DisableControl(int control_id);
  protected: void DispatchFormEvent(const base::string16& type, int control_id,
                                    const base::string16& value);
  protected: void EnableControl(int control_id, bool enable);
  protected: void FinishUpdateFromModel();
  protected: bool GetChecked(int control_id) const;
  protected: HWND GetDlgItem(int item_id) const;
  protected: base::string16 GetDlgItemText(int item_id) const;
  protected: virtual int GetTemplate() const = 0;
  protected: virtual void onCancel();
  protected: virtual bool onCommand(WPARAM wParam, LPARAM lParam);
  public: virtual bool OnIdle(int hint);
  protected: virtual bool onInitDialog() = 0;
  protected: virtual INT_PTR onMessage(UINT message, WPARAM wParam,
                                       LPARAM lParam);
  protected: virtual void onOk();
  public: void Realize();
  public: int SetCheckBox(int item_id, bool checked);
  public: void Show();
  private: void UpdateControlFromModel(HWND hwnd, int control_id);

  DISALLOW_COPY_AND_ASSIGN(DialogBox);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_forms_dialog_box_h)
