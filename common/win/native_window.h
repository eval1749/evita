// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_NATIVE_WINDOW_H_
#define COMMON_WIN_NATIVE_WINDOW_H_

#include <windows.h>
#include <memory>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {
namespace win {

class Point;
class Size;

class COMMON_EXPORT MessageDelegate {
 public:
  MessageDelegate();
  virtual ~MessageDelegate();

  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// NativeWindow
//
class COMMON_EXPORT NativeWindow {
 public:
  // Make destructor of NativeWindow for std::unique_ptr<T>. You should not
  // call |delete| for NativeWindow.
  virtual ~NativeWindow();

  operator HWND() const {
    DCHECK(hwnd_);
    return hwnd_;
  }

  bool operator==(const NativeWindow* other) const { return this == other; }
  bool operator==(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }
  bool operator!=(const NativeWindow* other) const { return this != other; }
  bool operator!=(HWND hwnd) const {
    DCHECK(hwnd_);
    return hwnd_ == hwnd;
  }

  bool CreateWindowEx(DWORD dwExStyle,
                      DWORD dwStyle,
                      const base::char16* title,
                      HWND parent_hwnd,
                      const Point& origin,
                      const Size& size);
  void Destroy();
  LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool IsRealized() const { return hwnd_ != nullptr; }

  LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) {
    return ::SendMessage(hwnd_, uMsg, wParam, lParam);
  }

  static std::unique_ptr<NativeWindow> Create(
      MessageDelegate* message_delegate);
  static std::unique_ptr<NativeWindow> Create();
  static void Init(HINSTANCE hInstance, HINSTANCE hResouce);
  static void Init(HINSTANCE hInstance);

 public:
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

 protected:
  // For MessageDelegate-less native window.
  NativeWindow();

  static NativeWindow* MapHwnToNativeWindow(HWND);

  // TODO(eval1749) |hwnd_| should be private.
  HWND hwnd_;

 private:
  explicit NativeWindow(MessageDelegate* message_delegate);

  static LRESULT CALLBACK WindowProc(HWND hwnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

  MessageDelegate* message_delegate_;

  DISALLOW_COPY_AND_ASSIGN(NativeWindow);
};

}  // namespace win
}  // namespace common

#endif  // COMMON_WIN_NATIVE_WINDOW_H_
