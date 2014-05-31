// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_native_window_h)
#define INCLUDE_common_win_native_window_h

#include <memory>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {
namespace win {

struct Point;
struct Size;

class COMMON_EXPORT MessageDelegate {
  public: MessageDelegate();
  public: virtual ~MessageDelegate();
  public: virtual LRESULT WindowProc(UINT message, WPARAM wParam,
                                     LPARAM lParam) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// NativeWindow
//
class COMMON_EXPORT NativeWindow {
  protected: HWND hwnd_;
  private: MessageDelegate* message_delegate_;

  private: explicit NativeWindow(MessageDelegate* message_delegate);

  // For MessageDelegate-less native window.
  protected: explicit NativeWindow();

  // Make destructor of NativeWindow for std::unique_ptr<T>. You should not
  // call |delete| for NativeWindow.
  public: virtual ~NativeWindow();

  public: operator HWND() const {
    DCHECK(hwnd_);
    return hwnd_;
  }

  public: bool operator==(const NativeWindow* other) const {
    return this == other;
  }

  public: bool operator==(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }

  public: bool operator!=(const NativeWindow* other) const {
    return this != other;
  }

  public: bool operator!=(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }

  // [C]
  public: static std::unique_ptr<NativeWindow> Create(
      MessageDelegate* message_delegate);
  public: static std::unique_ptr<NativeWindow> Create();
  public: bool CreateWindowEx(DWORD dwExStyle, DWORD dwStyle,
                              const base::char16* title, HWND parent_hwnd,
                              const Point& left_top,
                              const Size& size);

  // [D]
  public: void Destroy();
  public: LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

  // [I]
  public: static void Init(HINSTANCE hInstance, HINSTANCE hResouce);
  public: static void Init(HINSTANCE hInstance);
  public: virtual bool IsRealized() const { return hwnd_; }

  // [M]
  protected: static NativeWindow* MapHwnToNativeWindow(HWND);

  // [S]
  public: LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0,
                              LPARAM lParam = 0) {
    return ::SendMessage(hwnd_, uMsg, wParam, lParam);
  }

  // [W]
  private: static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam);

  public: virtual LRESULT WindowProc(UINT message, WPARAM wParam,
                                     LPARAM lParam);

  DISALLOW_COPY_AND_ASSIGN(NativeWindow);
};

} // namespace win
} // namespace common

#endif //!defined(INCLUDE_common_win_native_window_h)
