// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_visual_dialog_h)
#define INCLUDE_visual_dialog_h

#undef DialogBox

//////////////////////////////////////////////////////////////////////
//
// DialogBox
//
class DialogBox {
  private: HWND hwnd_;

  protected: DialogBox();
  public: virtual ~DialogBox();

  public: operator HWND() const { return hwnd_; }

  private: static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message,
                                              WPARAM wParam, LPARAM lParam);
  public: void DoModal(HWND hwnd);
  public: bool DoModeless(HWND hwnd = nullptr);
  protected: bool GetChecked(int control_id) const;
  protected: HWND GetDlgItem(int item_id) const;
  protected: virtual int GetTemplate() const = 0;
  protected: virtual bool onCommand(WPARAM wParam, LPARAM lParam);
  protected: virtual bool onInitDialog() = 0;
  protected: virtual void onOk();
  protected: virtual void onCancel();
  protected: virtual INT_PTR onMessage(UINT message, WPARAM wParam,
                                       LPARAM lParam);
  public: int SetCheckBox(int item_id, bool checked);
};

#endif //!defined(INCLUDE_visual_dialog_h)
