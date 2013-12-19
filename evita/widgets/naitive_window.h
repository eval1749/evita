// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_naitive_widget_h)
#define INCLUDE_widgets_naitive_window_h

#include "./li_util.h"
#include "widgets/widget.h"

#define MY_VK_CONTROL   0x100
#define MY_VK_SHIFT     0x200

namespace widgets {

//////////////////////////////////////////////////////////////////////
//
// NaitiveWindow
//
class NaitiveWindow {
  protected: HWND hwnd_;
  private: Widget* widget_;

  private: explicit NaitiveWindow(const Widget& widget);

  // For Widget-less naitive window.
  protected: explicit NaitiveWindow();

  // Make destructor of NaitiveWindow for OwnPtr<T>. You should not
  // call |delete| for NaitiveWindow.
  public: virtual ~NaitiveWindow();

  public: operator HWND() const {
    ASSERT(hwnd_);
    return hwnd_;
  }

  public: bool operator==(const NaitiveWindow* other) const {
    return this == other;
  }

  public: bool operator==(HWND hwnd) const {
    ASSERT(hwnd_);
    return hwnd_ == hwnd;
  }

  public: bool operator!=(const NaitiveWindow* other) const {
    return this != other;
  }

  public: bool operator!=(HWND hwnd) const {
    ASSERT(hwnd_);
    return hwnd_ == hwnd;
  }

  // [C]
  public: static std::unique_ptr<NaitiveWindow> Create(const Widget& widget);
  public: static std::unique_ptr<NaitiveWindow> Create();
  public: bool CreateWindowEx(DWORD dwExStyle, DWORD dwStyle,
                              const char16* title, HWND parent_hwnd,
                              const gfx::Point& left_top,
                              const gfx::Size& size);

  // [D]
  public: void Destroy();
  public: LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  // [I]
  public: static int Init();
  public: virtual bool IsRealized() const { return hwnd_; }

  // [M]
  protected: static NaitiveWindow* MapHwnToNaitiveWindow(HWND);

  // [S]
  public: LRESULT SendMessage(uint uMsg, WPARAM wParam = 0,
                              LPARAM lParam = 0) {
    return ::SendMessage(hwnd_, uMsg, wParam, lParam);
  }

  // [W]
  private: static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam);

  public: virtual LRESULT WindowProc(UINT message, WPARAM wParam,
                                     LPARAM lParam);

  DISALLOW_COPY_AND_ASSIGN(NaitiveWindow);
};

} // namespace widgets

#endif //!defined(INCLUDE_widgets_naitive_widget_h)
