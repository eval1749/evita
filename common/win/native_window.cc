// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "common/win/native_window.h"

#include "base/logging.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace win {

namespace {
ATOM s_window_class;
NativeWindow* s_creating_window;
HINSTANCE s_hInstance;
HINSTANCE s_hResource;
} // namespace

//////////////////////////////////////////////////////////////////////
//
// MessageDelegate
//
MessageDelegate::MessageDelegate() {
}

MessageDelegate::~MessageDelegate() {
}

//////////////////////////////////////////////////////////////////////
//
// NativeWindow
//
NativeWindow::NativeWindow(MessageDelegate* message_delegate)
    : hwnd_(nullptr),
      message_delegate_(message_delegate) {
}

NativeWindow::NativeWindow()
    : hwnd_(nullptr), message_delegate_(nullptr) {
}

NativeWindow::~NativeWindow() {
  #if DEBUG_DESTROY
    DVLOG(0) << "~NativeWindow " << this;
  #endif
  DCHECK(!hwnd_);
  DCHECK(!message_delegate_);
}

std::unique_ptr<NativeWindow> NativeWindow::Create(
    MessageDelegate* message_delegate) {
  return std::move(std::unique_ptr<NativeWindow>(
      new NativeWindow(message_delegate)));
}

std::unique_ptr<NativeWindow> NativeWindow::Create() {
  return std::unique_ptr<NativeWindow>();
}

bool NativeWindow::CreateWindowEx(DWORD dwExStyle, DWORD dwStyle,
                                  const base::char16* title, HWND parent_hwnd,
                                  const Point& origin,
                                  const Size& size) {
  DCHECK(!s_creating_window);
  s_creating_window = this;

  DCHECK(parent_hwnd == HWND_MESSAGE || !size.empty());

  if (!s_window_class) {
    WNDCLASSEXW wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_DBLCLKS | CS_BYTEALIGNCLIENT;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance  = s_hInstance;
    #pragma warning(suppress: 4302)
    wc.hIcon = ::LoadIconW(s_hResource, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor = nullptr;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"EvitaNativeWindow";
    #pragma warning(suppress: 4302)
    wc.hIconSm  = ::LoadIconW(s_hResource, MAKEINTRESOURCE(IDI_APPLICATION));
    s_window_class = ::RegisterClassExW(&wc);
    DCHECK(s_window_class);
  }

  // For child window, we need to assign unique id in parent window.
  // |ui::Widget::child_window_id()| exposes child window id.
  auto const child_id = dwStyle & WS_CHILD ?
      reinterpret_cast<HMENU>(this) : static_cast<HMENU>(nullptr);
  auto const hwnd = ::CreateWindowEx(dwExStyle, MAKEINTATOM(s_window_class),
                                     title, dwStyle, origin.x(), origin.y(),
                                     size.width(), size.height(),
                                     parent_hwnd, child_id,
                                     s_hInstance, 0);
  DCHECK(hwnd) << "CreateWindowEx err=" << ::GetLastError();
  return hwnd != nullptr;
}

void NativeWindow::Destroy() {
  DCHECK(IsRealized());
  ::DestroyWindow(hwnd_);
}

LRESULT NativeWindow::DefWindowProc(UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  return ::DefWindowProc(hwnd_, message, wParam, lParam);
}

void NativeWindow::Init(HINSTANCE hInstance, HINSTANCE hResource) {
  DCHECK(hInstance);
  DCHECK(hResource);
  DCHECK(!s_hInstance);
  DCHECK(!s_hResource);
  s_hInstance = hInstance;
  s_hResource = hResource;
}

void NativeWindow::Init(HINSTANCE hInstance) {
  Init(hInstance, hInstance);
}

NativeWindow* NativeWindow::MapHwnToNativeWindow(HWND const hwnd) {
  DCHECK(hwnd);
  return reinterpret_cast<NativeWindow*>(
    static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
}

LRESULT CALLBACK NativeWindow::WindowProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM  lParam) {

  if (auto const window = s_creating_window) {
    s_creating_window = nullptr;
    window->hwnd_ = hwnd;
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(window));
    return window->WindowProc(message, wParam, lParam);
  }

  auto const window = MapHwnToNativeWindow(hwnd);
  DCHECK(window);
  if (message == WM_NCDESTROY) {
    window->hwnd_ = nullptr;
    window->WindowProc(message, wParam, lParam);
    window->message_delegate_ = nullptr;
    delete window;
    return 0;
  }

  return window->WindowProc(message, wParam, lParam);
}

LRESULT NativeWindow::WindowProc(UINT message, WPARAM wParam,
                                  LPARAM lParam) {
  if (message_delegate_)
    return message_delegate_->WindowProc(message, wParam, lParam);
  return DefWindowProc(message, wParam, lParam);
}

} // namespace win
} // namespace common

