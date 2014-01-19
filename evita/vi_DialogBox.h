// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_visual_dialog_h)
#define INCLUDE_visual_dialog_h

#include <memory>

#include "base/strings/string16.h"
#include "evita/dom/forms/dialog_box_id.h"

typedef dom::DialogBoxId DialogBoxId;
const DialogBoxId kInvalidDialogBoxId = dom::kInvalidDialogBoxId;

namespace dom {
class Form;
}

#undef DialogBox

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

  protected: void DispatchTextEvent(const base::string16& type,
                                    int control_id);
  public: static DialogBox* FromDialogBoxId(DialogBoxId dialog_box_id);
  protected: bool GetChecked(int control_id) const;
  protected: HWND GetDlgItem(int item_id) const;
  protected: base::string16 GetDlgItemText(int item_id) const;
  protected: virtual int GetTemplate() const = 0;
  protected: virtual bool onCommand(WPARAM wParam, LPARAM lParam);
  protected: virtual bool onInitDialog() = 0;
  protected: virtual void onOk();
  protected: virtual void onCancel();
  protected: virtual INT_PTR onMessage(UINT message, WPARAM wParam,
                                       LPARAM lParam);
  public: void Realize(const dom::Form* form);
  public: int SetCheckBox(int item_id, bool checked);
  public: void Show();

  DISALLOW_COPY_AND_ASSIGN(DialogBox);
};

#endif //!defined(INCLUDE_visual_dialog_h)
