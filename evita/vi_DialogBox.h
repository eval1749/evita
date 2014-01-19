// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_visual_dialog_h)
#define INCLUDE_visual_dialog_h

#include "base/strings/string16.h"
#include "evita/dom/events/event_target_id.h"

typedef dom::EventTargetId DialogBoxId;
const DialogBoxId kInvalidDialogBoxId = dom::kInvalidEventTargetId;

#undef DialogBox

//////////////////////////////////////////////////////////////////////
//
// DialogBox
//
class DialogBox {
  private: DialogBoxId dialog_box_id_;
  private: HWND hwnd_;

  protected: DialogBox(DialogBoxId dialog_box_id);
  public: virtual ~DialogBox();

  public: operator HWND() const { return hwnd_; }
  public: DialogBoxId dialog_box_id() const { return dialog_box_id_; }

  private: static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message,
                                              WPARAM wParam, LPARAM lParam);

  public: void DoModal(HWND hwnd);
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
  public: void Realize();
  public: int SetCheckBox(int item_id, bool checked);
  public: void Show();
};

#endif //!defined(INCLUDE_visual_dialog_h)
