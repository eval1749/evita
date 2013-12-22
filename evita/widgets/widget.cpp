#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "widgets/widget.h"

#include "base/tree/ancestors_or_self.h"
#include "base/tree/descendants_or_self.h"
#include "widgets/container_widget.h"

#define DEBUG_FOCUS 0
#define DEBUG_RESIZE 0
#define DEBUG_SHOW 0

#if DEBUG_RESIZE
#include <string>
#endif

namespace widgets {

namespace {
class TopLevelWidget : public ContainerWidget {
  public: TopLevelWidget() {
  }
  private: virtual bool is_top_level() const { return true; }
  DISALLOW_COPY_AND_ASSIGN(TopLevelWidget);
};
}

Widget::Widget(std::unique_ptr<NaitiveWindow>&& naitive_window)
    : naitive_window_(std::move(naitive_window)),
      realized_(false),
      shown_(0) {
}

Widget::Widget()
    : Widget(NaitiveWindow::Create()) {
}

Widget::~Widget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  ASSERT(!naitive_window_);
}

bool Widget::has_focus() const {
  for (auto& runner: base::tree::ancestors_or_self(*this)) {
    if (auto const window = runner.naitive_window_.get()) {
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

ContainerWidget& Widget::top_level_widget() {
  static ContainerWidget* top_level_widget;
  if (!top_level_widget)
    top_level_widget = new TopLevelWidget();
  return *top_level_widget;
}

HWND Widget::AssociatedHwnd() const {
  for (auto& runner: base::tree::ancestors_or_self(*this)) {
    if (auto const window = runner.naitive_window_.get())
      return *window;
  }
  CAN_NOT_HAPPEN();
}

void Widget::CreateNaitiveWindow() const {
}

void Widget::Destroy() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  if (naitive_window_) {
    ::DestroyWindow(*naitive_window_.get());
    return;
  }
  WillDestroyWidget();
  container_widget().WillDestroyChildWidget(*this);
  container_widget().RemoveChild(*this);
  delete this;
}

void Widget::DidChangeParentWidget() {
}

void Widget::DidCreateNaitiveWindow() {
}

void Widget::DidDestroyNaitiveWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  ASSERT(!naitive_window_);
  // Since naitive window, which handles UI, is destroyed, this widget should
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
  if (naitive_window_)
    ::ShowWindow(*naitive_window_.get(), SW_HIDE);
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
  if (naitive_window_)
    return naitive_window_->DefWindowProc(message, wParam, lParam);
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
    if (auto const window = naitive_window())
      ::SetParent(*window, container_widget().AssociatedHwnd());
    DidChangeParentWidget();
    ResizeTo(rect);
    return;
  }

  realized_ = true;
  rect_ = rect;
  if (naitive_window_) {
    // On WM_CREATE, we call DidCreateNaitiveWindow() instead of DidRealized().
    CreateNaitiveWindow();
    return;
  }

  DidRealize();
  container_widget().DidRealizeChildWidget(*this);
}

void Widget::RealizeTopLevelWidget() {
  ASSERT(naitive_window_);
  ASSERT(!realized_);
  top_level_widget().AppendChild(*this);
  realized_ = true;
  CreateNaitiveWindow();
}

void Widget::ReleaseCapture() const {
  if (naitive_window_) {
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
  if (naitive_window_) {
    ::SetWindowPos(*naitive_window_.get(), nullptr, rect.left, rect.top,
                   rect.width(), rect.height(), SWP_NOACTIVATE);
  } else {
    rect_ = rect;
    DidResize();
  }
}

void Widget::SetCapture() const {
  if (naitive_window_) {
    ::SetCapture(*naitive_window_.get());
    return;
  }
  container_widget().SetCaptureTo(*this);
}

void Widget::SetFocus() {
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), shown_);
  #endif
  // This wieget might be hidden during creating window.
  if (naitive_window_) {
    ::SetFocus(*naitive_window_.get());
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
    if (auto const window = naitive_window())
      ::SetParent(*window, new_parent.AssociatedHwnd());
    DidChangeParentWidget();
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
  if (naitive_window_) {
    ::ShowWindow(*naitive_window_.get(), SW_SHOW);
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

void Widget::WillDestroyNaitiveWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("realized=%d show=%d " DEBUG_RECT_FORMAT "\n",
        realized_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
}

LRESULT Widget::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  ASSERT(naitive_window_);
  switch (message) {
    case WM_CREATE:
      if (reinterpret_cast<CREATESTRUCT*>(lParam)->style & WS_VISIBLE)
        ++shown_;
      ::GetClientRect(*naitive_window_.get(), &rect_);
      DidCreateNaitiveWindow();
      return 0;

    case WM_DESTROY:
      WillDestroyNaitiveWindow();
      return 0;

    case WM_KILLFOCUS:
      DidKillFocus();
      return 0;

    case WM_NCDESTROY:
      ASSERT(naitive_window_);
      // NaitiveWindow::WindowProc() will delete |naitive_window_|.
      naitive_window_.release();
      DidDestroyNaitiveWindow();
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
        for (auto& widget: base::tree::descendants_or_self(*this)) {
          widget.shown_ = 0;
        }
        return 0;
      }

      if (wp->flags & SWP_SHOWWINDOW) {
        for (auto& widget: base::tree::descendants_or_self(*this)) {
          widget.shown_ = 1;
        }
      }

      if (!(wp->flags & 0x10000000) && (wp->flags & SWP_NOSIZE))
        return 0;

      if (::IsIconic(*naitive_window_.get())) {
        // We don't take care miminize window.
        return 0;
      }

      ::GetClientRect(*naitive_window_.get(), &rect_);
      DidResize();
      return 0;
    }
  }

  return OnMessage(message, wParam, lParam);
}

} // namespace widgets

namespace logging {

base::string16 ToString16(const widgets::Widget& widget) {
  auto const class_name8 = widget.class_name();
  base::string16 class_name16(::lstrlenA(class_name8), ' ');
  auto runner8 = class_name8;
  for (auto& it: class_name16) {
    it = *runner8;
    ++runner8;
  }
  class_name16 += '@x';
  base::char16 address[20];
  ::wsprintfW(address, L"@%p",  &widget);
  class_name16 += address;
  return std::move(class_name16);
}

}  // namespace logging