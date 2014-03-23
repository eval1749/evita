// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/ui/widget.h"

#include <vector>

#include "common/adoptors/reverse.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "common/win/point_ostream.h"
#include "common/win/rect_ostream.h"
#include "common/win/win32_verify.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/event_ostream.h"
#include "evita/ui/root_widget.h"
#include "evita/ui/system_metrics.h"
#include "evita/ui/widget_ostream.h"

#define DEBUG_FOCUS 0
#define DEBUG_MOUSE 0
#define DEBUG_MOUSE_WHEEL 0
#define DEBUG_RESIZE 0
#define DEBUG_PAINT 0
#define DEBUG_SHOW 0

#if DEBUG_RESIZE
#include <string>
#endif

namespace ui {

namespace {
Widget* capture_widget;
Widget* focus_widget;
Widget* will_focus_widget;
bool we_have_active_focus;
}  // namespace

using ui::EventType;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
Widget::Widget(std::unique_ptr<NativeWindow>&& native_window)
    : native_window_(std::move(native_window)),
      shown_(0),
      state_(kNotRealized) {
}

Widget::Widget()
    : Widget(NativeWindow::Create()) {
}

Widget::~Widget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  DCHECK(!native_window_);
}

// Child window is assigned in |commonn::NativeWindow::CreateWindowEx()|.
UINT_PTR Widget::child_window_id() const {
  DCHECK(native_window());
  DCHECK(parent_node());
  return reinterpret_cast<UINT_PTR>(native_window());
}

bool Widget::has_active_focus() {
  // When modeless dialog has focus, ::GetFocus() returns it, but
  // we_have_active_focus is false.
  return we_have_active_focus;
}

bool Widget::has_focus() const {
  return focus_widget == this;
}

HWND Widget::AssociatedHwnd() const {
  for (auto runner : common::tree::ancestors_or_self(this)) {
    if (auto const window = runner->native_window_.get())
      return *window;
  }
  NOTREACHED();
  return nullptr;
}

void Widget::CreateNativeWindow() const {
}

void Widget::DestroyWidget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  if (state_ == kBeingDestroyed) {
    DCHECK(!native_window_);
    return;
  }
  if (native_window_) {
    ::DestroyWindow(*native_window_.get());
    return;
  }
  state_ = kBeingDestroyed;
  WillDestroyWidget();
  auto& parent_widget = container_widget();
  parent_widget.WillRemoveChildWidget(*this);
  if (capture_widget == this)
    ReleaseCapture();
  if (focus_widget == this) {
    focus_widget = nullptr;
    DidKillFocus();
  }
  while (first_child()) {
    first_child()->DestroyWidget();
  }
  parent_widget.RemoveChild(this);
  parent_widget.DidRemoveChildWidget(*this);
  state_ = kDestroyed;
  DidDestroyWidget();
}

void Widget::DidAddChildWidget(const Widget&) {
}

void Widget::DidChangeHierarchy() {
  for (auto child : child_nodes()) {
    child->DidChangeHierarchy();
  }
}

void Widget::DidCreateNativeWindow() {
  DidRealize();
}

void Widget::DidDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  DCHECK(!native_window_);
  // Since native window, which handles UI, is destroyed, this widget should
  // be destroyed too.
  DestroyWidget();
}

void Widget::DidDestroyWidget() {
  DCHECK_EQ(kDestroyed, state_);
  delete this;
}

void Widget::DidHide() {
}

void Widget::DidKillFocus() {
}

void Widget::DidRealize() {
  for (auto const child : child_nodes()) {
    child->RealizeWidget();
  }
}

void Widget::DidRealizeChildWidget(const Widget&) {
}

void Widget::DidRemoveChildWidget(const Widget&) {
}

void Widget::DidResize() {
}

void Widget::DidSetFocus() {
}

void Widget::DidShow() {
  for (auto child : child_nodes()) {
    child->Show();
  }
}

void Widget::DispatchPaintMessage() {
  Rect exposed_rect;
  if (!::GetUpdateRect(*native_window(), &exposed_rect, false))
    return;
  #if DEBUG_PAINT
    DEBUG_WIDGET_PRINTF("Start " DEBUG_RECT_FORMAT "\n", 
        DEBUG_RECT_ARG(exposed_rect));
  #endif
  OnPaint(exposed_rect);
  #if DEBUG_PAINT
    DEBUG_WIDGET_PRINTF("End " DEBUG_RECT_FORMAT "\n",
        DEBUG_RECT_ARG(exposed_rect));
  #endif

   for (auto child : child_nodes()) {
    if (!child->is_shown() || child->has_native_window())
      continue;
    auto const rect = exposed_rect.Intersect(child->rect());
    if (rect) {
      #if DEBUG_PAINT
        DEBUG_WIDGET_PRINTF("Start " DEBUG_WIDGET_FORMAT " focus=%d "
                            DEBUG_RECT_FORMAT "\n",
            DEBUG_WIDGET_ARG(&child),
            child.has_focus(),
            DEBUG_RECT_ARG(rect));
      #endif
      child->OnPaint(rect);
      #if DEBUG_PAINT
        DEBUG_WIDGET_PRINTF("End " DEBUG_RECT_FORMAT "\n",
            DEBUG_RECT_ARG(rect));
      #endif
    }
  }
}

HCURSOR Widget::GetCursorAt(const Point&) const {
  return ::LoadCursor(nullptr, IDC_ARROW);
}

Widget* Widget::GetFocusWidget() {
  return focus_widget;
}

Widget& Widget::GetHostWidget() const {
  for (auto runner : common::tree::ancestors_or_self(this)) {
    if (runner->native_window())
       return const_cast<Widget&>(*runner);
  }
  return *RootWidget::instance();
}

Widget* Widget::GetWidgetAt(const Point& point) const {
  // On release build by MSVS2013, using reverse() causes AV.
  // for (const auto& child: common::adoptors::reverse(child_nodes()))
  for (auto runner = last_child(); runner;
       runner = runner->previous_sibling()) {
    auto const child = runner;
    if (!child->is_shown())
      continue;
    if (child->rect().Contains(point)) {
      auto const child_child = child->GetWidgetAt(point);
      return child_child ? child_child : const_cast<Widget*>(child);
    }
  }
  return nullptr;
}

void Widget::Hide() {
  #if DEBUG_SHOW
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), shown_);
  #endif
  // Hide widgets in top to bottom == post order.
  for (auto child : common::adoptors::reverse(child_nodes())) {
    child->Hide();
  }
  shown_ = 0;
  if (native_window_)
    ::ShowWindow(*native_window_.get(), SW_HIDE);
  else
    DidHide();
}

void Widget::OnDraw(gfx::Graphics* gfx) {
  for (auto child : child_nodes()) {
    if (child->is_shown())
      child->OnDraw(gfx);
  }
}

void Widget::OnKeyPressed(const KeyboardEvent&) {
}

void Widget::OnKeyReleased(const KeyboardEvent&) {
}

void Widget::OnMousePressed(const MouseEvent&) {
}

void Widget::OnMouseReleased(const MouseEvent&) {
}

void Widget::OnMouseWheel(const MouseWheelEvent&) {
}

LRESULT Widget::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_MOUSEWHEEL) {
    OnMouseWheel(MouseWheelEvent(this, wParam, lParam));
    return 0;
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    auto event = MouseEvent::Create(this, message, wParam, lParam);
    #if DEBUG_MOUSE
      DVLOG_WIDGET(0) << "cap=" << capture_widget << " " << event;
    #endif
    if (event.event_type() == EventType::MouseMoved) {
        OnMouseMoved(event);
        return 0;
    }

    if (event.event_type() == EventType::MousePressed) {
      OnMousePressed(event);
      return 0;
    }

    if (event.event_type() == EventType::MouseReleased) {
      OnMouseReleased(event);
      return 0;
    }
  }
  if (native_window_)
    return native_window_->DefWindowProc(message, wParam, lParam);
  return container_widget().OnMessage(message, wParam, lParam);
}

void Widget::OnMouseMoved(const MouseEvent&) {
}

void Widget::OnPaint(const Rect) {
}

void Widget::OnScroll(int) {
}

void Widget::Realize(const Rect& rect) {
  DCHECK(parent_node());
  DCHECK(container_widget().is_realized());
  if (is_realized()) {
    if (auto const window = native_window())
      ::SetParent(*window, container_widget().AssociatedHwnd());
    DidChangeHierarchy();
    ResizeTo(rect);
    return;
  }

  state_ = kRealized;
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
  DCHECK(native_window_);
  DCHECK(!is_realized());
  RootWidget::instance()->AppendChild(this);
  state_ = kRealized;
  CreateNativeWindow();
}

// TODO(yosi) Widget::RealizeWidget() should be pure virutal.
void Widget::RealizeWidget() {
  DCHECK(parent_node());
  DCHECK(parent_node()->is_realized());
  DCHECK(!is_realized());

  state_ = kRealized;
  if (native_window_) {
    // On WM_CREATE, we call DidCreateNativeWindow() instead of DidRealized().
    CreateNativeWindow();
    return;
  }

  DidRealize();
  container_widget().DidRealizeChildWidget(*this);
}

void Widget::ReleaseCapture() {
  if (capture_widget != this)
    return;
  capture_widget = nullptr;
  ::ReleaseCapture();
}

void Widget::RequestFocus() {
  DCHECK(!will_focus_widget);
  #if DEBUG_FOCUS
    DVLOG_WIDGET(0) << " focus_hwnd=" << ::GetFocus() <<
        " focus=" << focus_widget;
  #endif
  // This wieget might be hidden during creating window.
  auto& host = GetHostWidget();
  if (::GetFocus() == *host.native_window()) {
    if (focus_widget == this)
      return;
    auto const last_focus_widget = focus_widget;
    focus_widget = this;
    if (last_focus_widget)
      last_focus_widget->DidKillFocus();
    focus_widget->DidSetFocus();
    return;
  }
  will_focus_widget = this;
  ::SetFocus(*host.native_window());
}

void Widget::ResizeTo(const Rect& rect) {
  DCHECK(state_ >= kNotRealized);

#if 0
  // TODO(yosi): We should enable this check.
  if (rect == rect_)
    return;
#endif
  if (is_realized() && native_window_) {
    ::SetWindowPos(*native_window_.get(), nullptr, rect.left, rect.top,
                   rect.width(), rect.height(), SWP_NOACTIVATE);
  } else {
    rect_ = rect;
    DidResize();
  }
}

void Widget::SchedulePaint() {
  SchedulePaintInRect(rect_);
}

void Widget::SchedulePaintInRect(const Rect& rect) {
  ::InvalidateRect(AssociatedHwnd(), &rect, true);
}


void Widget::SetCapture() {
  #if DEBUG_MOUSE
    DVLOG_WIDGET(0) << "cur=" << capture_widget << " new=" << this;
  #endif
  // We don't allow nested capture.
  if (capture_widget) {
    DVLOG_WIDGET(0) << "already captured by " << capture_widget;
    return;
  }
  ::SetCapture(*GetHostWidget().native_window());
  capture_widget = this;
}

bool Widget::SetCursor() {
  Point point;
  WIN32_VERIFY(::GetCursorPos(&point));
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, *native_window(),
                                 &point, 1));
  auto const widget = GetWidgetAt(point);
  if (!widget)
    return false;
  auto const hCursor = widget->GetCursorAt(point);
  if (!hCursor)
    return false;
  ::SetCursor(hCursor);
  return true;
}

void Widget::SetParentWidget(Widget* new_parent) {
  auto const old_parent = parent_node();
  if (new_parent == old_parent)
    return;
  if (old_parent) {
    old_parent->WillRemoveChildWidget(*this);
    old_parent->RemoveChild(this);
  }
  new_parent->AppendChild(this);
  if (new_parent->is_realized()) {
    if (auto const window = native_window())
      ::SetParent(*window, new_parent->AssociatedHwnd());
    DidChangeHierarchy();
  }
  if (old_parent)
    old_parent->DidRemoveChildWidget(*this);
  new_parent->DidAddChildWidget(*this);
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
    SchedulePaint();
  }

  // Show child in bottom to top == pre-order.
  for (auto child : child_nodes()) {
    child->Show();
  }
}

void Widget::WillDestroyWidget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
}

void Widget::WillDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  std::vector<Widget*> non_native_children;
  for (auto const child : child_nodes()) {
    if (!child->native_window())
      non_native_children.push_back(child);
  }
  for (auto const child : non_native_children) {
    child->DestroyWidget();
  }
}

void Widget::WillRemoveChildWidget(const Widget&) {
}

LRESULT Widget::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  DCHECK(native_window_);
  switch (message) {
    case WM_CAPTURECHANGED: {
      auto const new_capture = reinterpret_cast<HWND>(lParam);
      if (capture_widget && capture_widget->AssociatedHwnd() != new_capture) {
        DVLOG_WIDGET(0) << "Someone(" << new_capture <<
            ") gains the mouse capture.";
        capture_widget = nullptr;
      }
      return 0;
    }
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
      #if DEBUG_FOCUS
        DVLOG_WIDGET(0) << "WM_KILLFOCUS" <<
            " wParam=" << reinterpret_cast<HWND>(wParam) <<
            " cur=" << focus_widget <<
            " will=" << will_focus_widget;
      #endif
      if (auto widget = focus_widget) {
        focus_widget = nullptr;
        widget->DidKillFocus();
      }
      we_have_active_focus = false;
      return 0;

    case WM_NCDESTROY:
      DCHECK(native_window_);
      // NativeWidget will be deleted by itself.
      native_window_.release();
      DidDestroyNativeWindow();
      return 0;

    case WM_PAINT:
      DispatchPaintMessage();
      ::ValidateRect(*native_window(), nullptr);
      return 0;

    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT && SetCursor())
        return true;
      break;

    case WM_SETFOCUS:
      #if DEBUG_FOCUS
        DVLOG_WIDGET(0) << "WM_SETFOCUS" <<
            " wParam=" << reinterpret_cast<HWND>(wParam) <<
            " cur=" << focus_widget <<
            " will=" << will_focus_widget;
      #endif
      focus_widget = will_focus_widget ? will_focus_widget : this;
      will_focus_widget = nullptr;
      focus_widget->DidSetFocus();
      we_have_active_focus = true;
      return 0;

    case WM_SETTINGCHANGE:
      switch (wParam) {
        case SPI_SETICONTITLELOGFONT:
          SystemMetrics::instance()->NotifyChangeIconFont();
          break;
        case SPI_SETNONCLIENTMETRICS:
          SystemMetrics::instance()->NotifyChangeSystemMetrics();
          break;
      }
      return 0;

    case WM_SIZE:
      #if DEBUG_RESIZE
        DVLOG_WIDGET(0) << "WM_SIZE " << wParam << " " <<
            LOWORD(lParam) << "x" << HIWORD(lParam);
      #endif
      if (wParam != SIZE_MAXHIDE && wParam != SIZE_MINIMIZED) {
        ::GetClientRect(*native_window_.get(), &rect_);
        DidResize();
      }
      return 0;

    case WM_SYSCOLORCHANGE:
      SystemMetrics::instance()->NotifyChangeSystemColor();
      return 0;

    case WM_VSCROLL: {
      auto const widget = reinterpret_cast<Widget*>(::GetWindowLongPtr(
          reinterpret_cast<HWND>(lParam), GWLP_ID));
      widget->OnScroll(static_cast<int>(LOWORD(wParam)));
      return 0;
    }

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
        DVLOG_WIDGET(0) << "WM_WINDOWPOSCHANGED " <<
            "(" << wp->x << "," << wp->y << ")" <<
            "+" << wp->cx << "x" << wp->cy <<
            " insertAfter" << wp->hwndInsertAfter <<
            " flags=" << std::hex << wp->flags << flags;
      }
      #endif

      if (wp->flags & SWP_HIDEWINDOW) {
        // We don't take care hidden window.
        for (auto widget : common::tree::descendants_or_self(this)) {
          widget->shown_ = 0;
        }
        return 0;
      }

      if (wp->flags & SWP_SHOWWINDOW) {
        for (auto widget : common::tree::descendants_or_self(this)) {
          widget->shown_ = 1;
        }
      }

      if (wp->flags & SWP_NOSIZE)
        return 0;

      if (::IsIconic(*native_window_.get())) {
        // We don't care minimize window.
        return 0;
      }

      ::GetClientRect(*native_window_.get(), &rect_);
      DidResize();
      return 0;
    }
  }

  if (focus_widget) {
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST ||
        message >= WM_IME_STARTCOMPOSITION && message<= WM_IME_KEYLAST) {
      auto event = KeyboardEvent::Create(message, wParam, lParam);
      if (event.event_type() == EventType::KeyPressed) {
        focus_widget->OnKeyPressed(event);
        return 0;
      }
      if (event.event_type() == EventType::KeyReleased) {
        focus_widget->OnKeyReleased(event);
        return 0;
      }
      return focus_widget->OnMessage(message, wParam, lParam);
    }
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    if (capture_widget)
      return capture_widget->OnMessage(message, wParam, lParam);

    // Note: We send WM_MOUSEWHEEL message to a widget under mouse pointer
    // rather than active widget.
    Point point(MAKEPOINTS(lParam));
    if (message == WM_MOUSEWHEEL) {
      WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, *native_window(),
                                     &point, 1));
    }

    if (auto child = GetWidgetAt(point)) {
      #if DEBUG_MOUSE_WHEEL
        if (message == WM_MOUSEWHEEL) {
          DVLOG_WIDGET(0) << "WM_MOUSEWHEEL " << child << " at " <<
              point << " in " << child->rect();
        }
      #endif
      return child->OnMessage(message, wParam, lParam);
    }

    #if DEBUG_MOUSE_WHEEL
      if (message == WM_MOUSEWHEEL) {
        DVLOG_WIDGET(0) << "WM_MOUSEWHEEL " << this << " at " << point;
      }
    #endif
  }

  return OnMessage(message, wParam, lParam);
}

} // namespace ui
