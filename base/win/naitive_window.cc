// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "base/win/naitive_window.h"

#include "base/win/point.h"
#include "base/win/size.h"

namespace base {
namespace win {

namespace {
ATOM s_window_class;
NaitiveWindow* s_creating_window;
HINSTANCE s_hInstance;
HINSTANCE s_hResource;
} // namespace

//////////////////////////////////////////////////////////////////////
//
// MessageDelegate
//
MessageDelegate::~MessageDelegate() {
}

//////////////////////////////////////////////////////////////////////
//
// NaitiveWindow
//
NaitiveWindow::NaitiveWindow(const MessageDelegate& message_delegate)
    : hwnd_(nullptr),
      message_delegate_(const_cast<MessageDelegate*>(&message_delegate)) {
}

NaitiveWindow::NaitiveWindow()
    : hwnd_(nullptr), message_delegate_(nullptr) {
}

NaitiveWindow::~NaitiveWindow() {
  #if DEBUG_DESTROY
    DEBUG_PRINTF("%p " DEBUG_MESSAGE_DELEGATE_FORMAT "\n", this,
        DEBUG_MESSAGE_DELEGATE_ARG(message_delegate_));
  #endif
  DCHECK(!hwnd_);
  DCHECK(!message_delegate_);
}

std::unique_ptr<NaitiveWindow> NaitiveWindow::Create(
    const MessageDelegate& message_delegate) {
  return std::move(std::unique_ptr<NaitiveWindow>(
      new NaitiveWindow(message_delegate)));
}

std::unique_ptr<NaitiveWindow> NaitiveWindow::Create() {
  return std::unique_ptr<NaitiveWindow>();
}

bool NaitiveWindow::CreateWindowEx(DWORD dwExStyle, DWORD dwStyle,
                                  const char16* title, HWND parent_hwnd, 
                                  const Point& left_top,
                                  const Size& size) {
  DCHECK(!s_creating_window);
  s_creating_window = this;

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
    wc.lpszClassName = L"EvitaNaitiveWindow";
    #pragma warning(suppress: 4302)
    wc.hIconSm  = ::LoadIconW(s_hResource, MAKEINTRESOURCE(IDI_APPLICATION));
    s_window_class = ::RegisterClassExW(&wc);
    DCHECK(s_window_class);
  }

  return ::CreateWindowEx(dwExStyle, MAKEINTATOM(s_window_class), title,
                          dwStyle, left_top.x, left_top.y, size.cx, size.cy,
                          parent_hwnd, nullptr, s_hInstance, 0);
}

void NaitiveWindow::Destroy() {
  DCHECK(IsRealized());
  ::DestroyWindow(hwnd_);
}

LRESULT NaitiveWindow::DefWindowProc(UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  return ::DefWindowProc(hwnd_, message, wParam, lParam);
}

void NaitiveWindow::Init(HINSTANCE hInstance, HINSTANCE hResource) {
  DCHECK(hInstance);
  DCHECK(hResource);
  DCHECK(!s_hInstance);
  DCHECK(!s_hResource);
  s_hInstance = hInstance;
  s_hResource = hResource;
}

void NaitiveWindow::Init(HINSTANCE hInstance) {
  Init(hInstance, hInstance);
}

NaitiveWindow* NaitiveWindow::MapHwnToNaitiveWindow(HWND const hwnd) {
  DCHECK(hwnd);
  return reinterpret_cast<NaitiveWindow*>(
    static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
}

LRESULT CALLBACK NaitiveWindow::WindowProc(HWND hwnd, UINT message,
                                           WPARAM wParam, LPARAM  lParam) {

  if (auto const window = s_creating_window) {
    s_creating_window = nullptr;
    window->hwnd_ = hwnd;
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        static_cast<LONG>(reinterpret_cast<LONG_PTR>(window)));
    return window->WindowProc(message, wParam, lParam);
  }

  auto const window = MapHwnToNaitiveWindow(hwnd);
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

LRESULT NaitiveWindow::WindowProc(UINT message, WPARAM wParam,
                                  LPARAM lParam) {
  if (message_delegate_)
    return message_delegate_->WindowProc(message, wParam, lParam);
  return DefWindowProc(message, wParam, lParam);
}

} // namespace win
} // namespace base

