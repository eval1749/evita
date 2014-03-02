// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_dialog_box_h)
#define INCLUDE_evita_views_forms_dialog_box_h

#include <memory>

#include "base/strings/string16.h"
#include "evita/dom/forms/dialog_box_id.h"

typedef dom::DialogBoxId DialogBoxId;
const DialogBoxId kInvalidDialogBoxId = dom::kInvalidDialogBoxId;

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
  private: class Model;

  private: DialogBoxId dialog_box_id_;
  private: HWND hwnd_;
  private: std::unique_ptr<Model> model_;

  protected: DialogBox(DialogBoxId dialog_box_id);
  public: virtual ~DialogBox();

  public: operator HWND() const { return hwnd_; }
  public: DialogBoxId dialog_box_id() const { return dialog_box_id_; }

  private: static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message,
                                              WPARAM wParam, LPARAM lParam);
  protected: void DispatchFormEvent(const base::string16& type, int control_id,
                                    const base::string16& value);
  public: static DialogBox* FromDialogBoxId(DialogBoxId dialog_box_id);
  protected: bool GetChecked(int control_id) const;
  protected: HWND GetDlgItem(int item_id) const;
  protected: base::string16 GetDlgItemText(int item_id) const;
  protected: virtual int GetTemplate() const = 0;
  protected: virtual void onCancel();
  protected: virtual bool onCommand(WPARAM wParam, LPARAM lParam);
  protected: virtual bool onInitDialog() = 0;
  protected: virtual INT_PTR onMessage(UINT message, WPARAM wParam,
                                       LPARAM lParam);
  protected: virtual void onOk();
  public: void Realize(const dom::Form* form);
  public: int SetCheckBox(int item_id, bool checked);
  public: void Show();

  DISALLOW_COPY_AND_ASSIGN(DialogBox);
};

}  // namespace views

#endif //!defined(INCLUDE_visual_dialog_h)
