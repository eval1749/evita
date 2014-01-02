#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "widgets/widget.h"

#include <unordered_map>
#include <vector>

#include "common/adoptors/reverse.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/widgets/root_widget.h"

#define DEBUG_FOCUS 0
#define DEBUG_IDLE 0
#define DEBUG_MOUSE 0
#define DEBUG_RESIZE 0
#define DEBUG_PAINT 0
#define DEBUG_SHOW 0

#if DEBUG_RESIZE
#include <string>
#endif

namespace widgets {

namespace {
Widget* capture_widget;
Widget* focus_widget;
Widget* will_focus_widget;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WidgetIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
class WidgetIdMapper : public common::Singleton<WidgetIdMapper> {
  friend class common::Singleton<WidgetIdMapper>;

  private: typedef widgets::WidgetId WidgetId;

  private: std::unordered_map<WidgetId, Widget*> map_;

  private: WidgetIdMapper() = default;
  public: ~WidgetIdMapper() = default;

  public: void DidDestroyDomWindow(WidgetId widget_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
    auto it = map_.find(widget_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a window for WidgetId " << widget_id <<
        " in WidgetIdMap?";
      return;
    }
    map_.erase(it);
  }

  public: Widget* Find(WidgetId widget_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWidgetId, widget_id);
    auto it = map_.find(widget_id);
    return it == map_.end() ? nullptr : it->second;
  }

  public: WidgetId Register(Widget* widget) {
    ASSERT_CALLED_ON_UI_THREAD();
    auto const widget_id = widget->widget_id();
    DCHECK_NE(kInvalidWidgetId, widget_id);
    DCHECK_EQ(0u, map_.count(widget_id));
    map_[widget_id] = widget;
    return widget_id;
  }

  public: void Unregister(WidgetId widget_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
    map_[widget_id] = nullptr;
  }
};

//////////////////////////////////////////////////////////////////////
//
// Widget
//
Widget::Widget(std::unique_ptr<NativeWindow>&& native_window,
               WidgetId widget_id)
    : native_window_(std::move(native_window)),
      shown_(0),
      state_(kNotRealized),
      widget_id_(widget_id) {
  if (widget_id != kInvalidWidgetId)
    WidgetIdMapper::instance()->Register(this);
}

Widget::Widget(WidgetId widget_id)
    : Widget(NativeWindow::Create(), widget_id) {
}

Widget::~Widget() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  DCHECK(!native_window_);
  if (widget_id_ != widgets::kInvalidWidgetId) {
    WidgetIdMapper::instance()->Unregister(widget_id_);
    Application::instance()->view_event_handler()->
        DidDestroyWidget(widget_id_);
  }
}

bool Widget::has_focus() const {
  return focus_widget == this;
}

HWND Widget::AssociatedHwnd() const {
  for (auto runner : common::tree::ancestors_or_self(this)) {
    if (auto const window = runner->native_window_.get())
      return *window;
  }
  CAN_NOT_HAPPEN();
}

void Widget::CreateNativeWindow() const {
}

void Widget::Destroy() {
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

void Widget::DidDestroyDomWindow(WidgetId widget_id) {
  WidgetIdMapper::instance()->DidDestroyDomWindow(widget_id);
}

void Widget::DidDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DEBUG_WIDGET_PRINTF("state=%d show=%d " DEBUG_RECT_FORMAT "\n",
        state_, shown_, DEBUG_RECT_ARG(rect_));
  #endif
  DCHECK(!native_window_);
  // Since native window, which handles UI, is destroyed, this widget should
  // be destroyed too.
  Destroy();
}

void Widget::DidDestroyWidget() {
  delete this;
}

void Widget::DidHide() {
}

void Widget::DidKillFocus() {
}

void Widget::DidRealize() {
  Application::instance()->view_event_handler()->DidRealizeWidget(widget_id_);
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

Widget* Widget::FromWidgetId(WidgetId widget_id) {
  return WidgetIdMapper::instance()->Find(widget_id);
}

HCURSOR Widget::GetCursorAt(const Point&) const {
  return nullptr;
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

bool Widget::OnIdle(uint idle_count) {
  #if DEBUG_IDLE
    DEBUG_WIDGET_PRINTF("count=%d\n", idle_count);
  #endif
  auto more = false;
  for (auto child : child_nodes()) {
    if (child->OnIdle(idle_count))
      more = true;
  }
  return more;
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
  DCHECK_EQ(capture_widget, this);
  capture_widget = nullptr;
  ::ReleaseCapture();
}

void Widget::ResizeTo(const Rect& rect) {
  DCHECK(state_ >= kNotRealized);

#if 0
  // TODO(yosi): We should enable this check.
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

void Widget::SetCapture() {
  #if DEBUG_MOUSE
    DEBUG_WIDGET_PRINTF("capture=" DEBUG_WIDGET_FORMAT " new="
                        DEBUG_WIDGET_FORMAT "\n",
        DEBUG_WIDGET_ARG(capture_widget), DEBUG_WIDGET_ARG(&widget));
  #endif
  // We don't allow nested capture.
  DCHECK(!capture_widget);
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

void Widget::SetFocus() {
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

void Widget::SetParentWidget(const Widget& new_parent) {
  auto const old_parent = parent_node();
  if (new_parent == old_parent)
    return;
  if (old_parent) {
    old_parent->WillRemoveChildWidget(*this);
    old_parent->RemoveChild(this);
  }
  const_cast<Widget&>(new_parent).AppendChild(this);
  if (new_parent.is_realized()) {
    if (auto const window = native_window())
      ::SetParent(*window, new_parent.AssociatedHwnd());
    DidChangeHierarchy();
  }
  if (old_parent)
    old_parent->DidRemoveChildWidget(*this);
  const_cast<Widget&>(new_parent).DidAddChildWidget(*this);
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
    child->Destroy();
  }
}

void Widget::WillRemoveChildWidget(const Widget&) {
}

LRESULT Widget::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  DCHECK(native_window_);
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

  if (focus_widget) {
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
      return focus_widget->OnMessage(message, wParam, lParam);
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    if (capture_widget) {
      capture_widget->OnMessage(message, wParam, lParam);
    } else {
      // Note: We send WM_MOUSEWHEEL message to a widget under mouse pointer
      // rather than active widget.
      Point point(MAKEPOINTS(lParam));
      if (message == WM_MOUSEWHEEL) {
        WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, *native_window(),
                                       &point, 1));
      }
      #if DEBUG_MOUSE
        if (message == WM_MOUSEWHEEL) {
          DEBUG_WIDGET_PRINTF("WM_MOUSEWHEEL " DEBUG_POINT_FORMAT "\n",
              DEBUG_POINT_ARG(point));
        }
      #endif
      if (auto child = GetWidgetAt(point)) {
        #if DEBUG_MOUSE
          if (message == WM_MOUSEWHEEL) {
            DEBUG_WIDGET_PRINTF("WM_MOUSEWHEEL to "
                DEBUG_WIDGET_FORMAT " " DEBUG_RECT_FORMAT "\n",
                DEBUG_WIDGET_ARG(child), child->rect());
          }
        #endif
        return child->OnMessage(message, wParam, lParam);
      }
    }
  }

  return OnMessage(message, wParam, lParam);
}

} // namespace widgets

std::ostream& operator<<(std::ostream& out, const widgets::Widget& widget) {
  const auto& rect = widget.rect();
  out << "{" << widget.class_name() << "@" << std::hex <<
    reinterpret_cast<uintptr_t>(&widget) << std::dec << " (" << rect.left <<
    "," << rect.top << ")+(" << rect.width() << "x" << rect.height() << ")}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const widgets::Widget* widget) {
  if (widget)
    return out << *widget;
  return out << "null";
}
