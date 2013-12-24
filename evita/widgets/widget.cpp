#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "widgets/widget.h"

#include "common/tree/ancestors_or_self.h"
#include "common/tree/descendants_or_self.h"
#include "evita/widgets/container_widget.h"
#include "evita/widgets/root_widget.h"

#define DEBUG_FOCUS 0
#define DEBUG_RESIZE 0
#define DEBUG_SHOW 0

#if DEBUG_RESIZE
#include <string>
#endif

namespace widgets {

Widget::Widget(std::unique_ptr<NativeWindow>&& native_window)
    : native_window_(std::move(native_window)),
      realized_(false),
      shown_(0) {
}

Widget::Widget()
    : Widget(NativeWindow::Create()) {
}

Widget::~Widget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  ASSERT(!native_window_);
}

bool Widget::has_focus() const {
  for (auto& runner: common::tree::ancestors_or_self(*this)) {
    if (auto const window = runner.native_window_.get()) {
      if (::GetFocus() != *window)
        return false;
      if (runner == this)
        return true;
      if (auto const container = runner.ToContainer())
        return container->focus_widget() == this;
    }
  }
  return false;
}

HWND Widget::AssociatedHwnd() const {
  for (auto& runner: common::tree::ancestors_or_self(*this)) {
    if (auto const window = runner.native_window_.get())
      return *window;
  }
  CAN_NOT_HAPPEN();
}

void Widget::CreateNativeWindow() const {
}

void Widget::Destroy() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  if (native_window_) {
    ::DestroyWindow(*native_window_.get());
    return;
  }
  WillDestroyWidget();
  container_widget().WillDestroyChildWidget(*this);
  container_widget().RemoveChild(*this);
  delete this;
}

void Widget::DidChangeHierarchy() {
}

void Widget::DidCreateNativeWindow() {
}

void Widget::DidDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  ASSERT(!native_window_);
  // Since native window, which handles UI, is destroyed, this widget should
  // be destroyed too.
  Destroy();
}

HCURSOR Widget::GetCursorAt(const Point&) const {
  return nullptr;
}

void Widget::Hide() {
  #if DEBUG_SHOW
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), shown_);
  #endif
  shown_ = 0;
  if (native_window_)
    ::ShowWindow(*native_window_.get(), SW_HIDE);
  else
    DidHide();
}

bool Widget::OnIdle(uint) {
  return false;
}

void Widget::OnLeftButtonDown(uint, const Point&) {
}

void Widget::OnLeftButtonUp(uint, const Point&) {
}

LRESULT Widget::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_LBUTTONDOWN:
      OnLeftButtonDown(static_cast<uint>(wParam), 
                       Point(MAKEPOINTS(lParam)));
      return 0;

    case WM_LBUTTONUP:
      OnLeftButtonUp(static_cast<uint>(wParam), 
                     Point(MAKEPOINTS(lParam)));
      return 0;

    case WM_MOUSEMOVE:
      OnMouseMove(static_cast<uint>(wParam), 
                  Point(MAKEPOINTS(lParam)));
      return 0;
  }
  if (native_window_)
    return native_window_->DefWindowProc(message, wParam, lParam);
  return container_widget().OnMessage(message, wParam, lParam);
}

void Widget::OnMouseMove(uint, const Point&) {
}

void Widget::OnPaint(const Rect) {
}

void Widget::Realize(const Rect& rect) {
  ASSERT(parent_node());
  ASSERT(container_widget().is_realized());
  if (is_realized()) {
    if (auto const window = native_window())
      ::SetParent(*window, container_widget().AssociatedHwnd());
    DidChangeHierarchy();
    ResizeTo(rect);
    return;
  }

  realized_ = true;
  rect_ = rect;
  if (native_window_) {
    // On WM_CREATE, we call DidCreateNativeWindow() instead of DidRealized().
    CreateNativeWindow();
    return;
  }

  DidRealize();
  container_widget().DidRealizeChildWidget(*this);
}

void Widget::RealizeTopLevelWidget() {
  ASSERT(native_window_);
  ASSERT(!realized_);
  RootWidget::instance().AppendChild(*this);
  realized_ = true;
  CreateNativeWindow();
}

void Widget::ReleaseCapture() const {
  if (native_window_) {
    ::ReleaseCapture();
    return;
  }
  container_widget().ReleaseCaptureFrom(*this);
}

void Widget::ResizeTo(const Rect& rect) {
  ASSERT(realized_);

#if 0
  // TODO: We should enable this check.
  if (rect == rect_)
    return;
#endif
  if (native_window_) {
    ::SetWindowPos(*native_window_.get(), nullptr, rect.left, rect.top,
                   rect.width(), rect.height(), SWP_NOACTIVATE);
  } else {
    rect_ = rect;
    DidResize();
  }
}

void Widget::SetCapture() const {
  if (native_window_) {
    ::SetCapture(*native_window_.get());
    return;
  }
  container_widget().SetCaptureTo(*this);
}

void Widget::SetFocus() {
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), shown_);
  #endif
  // This wieget might be hidden during creating window.
  if (native_window_) {
    ::SetFocus(*native_window_.get());
    return;
  }
  container_widget().SetFocusTo(*this);
}

void Widget::SetParentWidget(const ContainerWidget& new_parent) {
  auto const old_parent = parent_node();
  if (new_parent == old_parent)
    return;
  if (old_parent) {
    old_parent->WillRemoveChildWidget(*this);
    old_parent->RemoveChild(*this);
  }
  const_cast<ContainerWidget&>(new_parent).AppendChild(*this);
  if (new_parent.is_realized()) {
    if (auto const window = native_window())
      ::SetParent(*window, new_parent.AssociatedHwnd());
    DidChangeHierarchy();
  }
  if (old_parent)
    old_parent->DidRemoveChildWidget(*this);
  const_cast<ContainerWidget&>(new_parent).DidAddChildWidget(*this);
}

void Widget::Show() {
  #if DEBUG_SHOW
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), shown_);
  #endif
  ++shown_;
  if (native_window_) {
    ::ShowWindow(*native_window_.get(), SW_SHOW);
  } else if (shown_ == 1) {
    DidShow();
    OnPaint(rect());
  }
}

void Widget::WillDestroyWidget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
}

void Widget::WillDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
}

LRESULT Widget::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  ASSERT(native_window_);
  switch (message) {
    case WM_CREATE:
      if (reinterpret_cast<CREATESTRUCT*>(lParam)->style & WS_VISIBLE)
        ++shown_;
      ::GetClientRect(*native_window_.get(), &rect_);
      DidCreateNativeWindow();
      return 0;

    case WM_DESTROY:
      WillDestroyNativeWindow();
      return 0;

    case WM_KILLFOCUS:
      DidKillFocus();
      return 0;

    case WM_NCDESTROY:
      ASSERT(native_window_);
      // NativeWindow::WindowProc() will delete |native_window_|.
      native_window_.release();
      DidDestroyNativeWindow();
      return 0;

    case WM_SETFOCUS:
      DidSetFocus();
      return 0;

    case WM_WINDOWPOSCHANGED: {
      // DefWindowProc sents WM_SIZE and WM_MOVE, so handling
      // WM_WINDPOSCHANGED is faster than DefWindowProc.
      // undocumented SWP flags. See http://www.winehq.org.
      #if !defined(SWP_NOCLIENTSIZE)
          #define SWP_NOCLIENTSIZE    0x0800
          #define SWP_NOCLIENTMOVE    0x1000
      #endif // !defined(SWP_NOCLIENTSIZE)
      // Create   0x10001843  NOCLIENTMOVE NOCLIENTSIZE SHOWWINDOW NOMOV NOSIZE
      // Minimize 0x00008130
      // Restore  0x00008124
      // Move     0x00000A15
      // Destroy  0x20001897  NOCLIENTMOVE NOCLIENTSIZE HIDEWINDOW NOACTIVATE
      //                      NOZORDER NOMOVE NOSIZE
      //if (wp->flags & SWP_NOSIZE) return 0;

      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);

      #if DEBUG_RESIZE
      {
        std::string flags;
        #define CHECK_FLAG(name) \
          if (wp->flags & SWP_ ##name) flags += " " #name;
        CHECK_FLAG(NOSIZE) // 0x0001
        CHECK_FLAG(NOMOVE) // 0x0002
        CHECK_FLAG(NOZORDER) // 0x0004
        CHECK_FLAG(NOREDRAW) // 0x0008
        CHECK_FLAG(NOACTIVATE) // 0x0010
        CHECK_FLAG(FRAMECHANGED) // 0x0020
        CHECK_FLAG(SHOWWINDOW) // 0x0040
        CHECK_FLAG(HIDEWINDOW) // 0x0080
        CHECK_FLAG(NOCOPYBITS) // 0x0100
        CHECK_FLAG(DEFERERASE) // 0x0200
        CHECK_FLAG(NOSENDCHANGING) // 0x0400
        CHECK_FLAG(NOCLIENTSIZE) // 0x0800
        CHECK_FLAG(NOCLIENTMOVE) // 0x1000
        CHECK_FLAG(DEFERERASE) // 0x2000
        CHECK_FLAG(ASYNCWINDOWPOS) // 0x4000
        DEBUG_WIDGET_PRINTF("WM_WINDOWPOSCHANGED (%d,%d)+%dx%d %08X%s\n",
            wp->x, wp->y, wp->cx, wp->cy, wp->flags, flags.c_str());
      }
      #endif

      if (wp->flags & SWP_HIDEWINDOW) {
        // We don't take care hidden window.
        for (auto& widget: common::tree::descendants_or_self(*this)) {
          widget.shown_ = 0;
        }
        return 0;
      }

      if (wp->flags & SWP_SHOWWINDOW) {
        for (auto& widget: common::tree::descendants_or_self(*this)) {
          widget.shown_ = 1;
        }
      }

      if (!(wp->flags & 0x10000000) && (wp->flags & SWP_NOSIZE))
        return 0;

      if (::IsIconic(*native_window_.get())) {
        // We don't take care miminize window.
        return 0;
      }

      ::GetClientRect(*native_window_.get(), &rect_);
      DidResize();
      return 0;
    }
  }

  return OnMessage(message, wParam, lParam);
}

} // namespace widgets

std::ostream& operator<<(std::ostream& out, const widgets::Widget& widget) {
  out << widget.class_name() << " 0x" << std::hex <<
    reinterpret_cast<uintptr_t>(&widget);
  return out;
}
