// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/ui/widget.h"

#include "base/trace_event/trace_event.h"
#include "common/adopters/reverse.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "common/win/win32_verify.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/ui/base/ime/text_input_client_win.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/mouse_click_tracker.h"
#include "evita/ui/events/native_event_win.h"
#include "evita/ui/root_widget.h"
#include "evita/ui/system_metrics.h"

namespace ui {

namespace {

Widget* capture_widget;
Widget* hover_widget;

gfx::Point GetCursorPoint() {
  POINT cursor_point;
  WIN32_VERIFY(::GetCursorPos(&cursor_point));
  return gfx::Point(cursor_point);
}

}  // namespace

using ui::EventType;

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
class Widget::HitTestResult final {
 public:
  HitTestResult(const Widget* widget, const gfx::Point& local_point);
  HitTestResult(const HitTestResult& other);
  HitTestResult();
  ~HitTestResult() = default;

  explicit operator bool() const { return widget_ != nullptr; }

  const gfx::Point& local_point() const { return local_point_; }
  Widget* widget() const { return widget_; }

 private:
  Point local_point_;
  Widget* widget_;
};

Widget::HitTestResult::HitTestResult(const Widget* widget,
                                     const gfx::Point& local_point)
    : local_point_(local_point), widget_(const_cast<Widget*>(widget)) {}

Widget::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.widget_, other.local_point_) {}

Widget::HitTestResult::HitTestResult() : HitTestResult(nullptr, Point()) {}

//////////////////////////////////////////////////////////////////////
//
// Widget
//
Widget::Widget(std::unique_ptr<NativeWindow> native_window)
    : native_window_(std::move(native_window)),
      owned_by_client_(false),
      visible_(0),
      state_(kNotRealized) {}

Widget::Widget() : Widget(NativeWindow::Create()) {}

Widget::~Widget() {
  DCHECK(!native_window_);
}

// Child window is assigned in |common::NativeWindow::CreateWindowEx()|.
UINT_PTR Widget::child_window_id() const {
  DCHECK(native_window());
  DCHECK(parent_node());
  return reinterpret_cast<UINT_PTR>(native_window());
}

const Widget* Widget::container_widget() const {
  DCHECK(parent_node());
  return parent_node();
}

Widget* Widget::container_widget() {
  DCHECK(parent_node());
  return parent_node();
}

bool Widget::has_focus() const {
  return FocusController::instance()->focus_widget() == this;
}

bool Widget::has_native_focus() const {
  return AssociatedHwnd() == ::GetFocus();
}

HWND Widget::AssociatedHwnd() const {
  for (auto runner : common::tree::ancestors_or_self(this)) {
    if (auto const window = runner->native_window_.get())
      return *window;
  }
  NOTREACHED();
  return nullptr;
}

void Widget::CreateNativeWindow() const {}

void Widget::DestroyWidget() {
  if (state_ == kBeingDestroyed)
    return;
  state_ = kBeingDestroyed;
  if (native_window_) {
    ::DestroyWindow(*native_window_.get());
    return;
  }
  WillDestroyWidget();
  state_ = kDestroyed;
  DidDestroyWidget();
}

void Widget::DidActivate() {}

void Widget::DidAddChildWidget(Widget* widget) {}

void Widget::DidChangeHierarchy() {
  for (auto child : child_nodes())
    child->DidChangeHierarchy();
}

void Widget::DidChangeChildVisibility(Widget* widget) {}

void Widget::DidDestroyNativeWindow() {
  DCHECK_EQ(kBeingDestroyed, state_);
  DCHECK(native_window_);
  // NativeWidget will be deleted by itself.
  native_window_.release();
  state_ = kDestroyed;
  // Since native window, which handles UI, is destroyed, this widget should
  // be destroyed too.
  DidDestroyWidget();
}

void Widget::DidDestroyWidget() {
  DCHECK_EQ(kDestroyed, state_);
  while (first_child())
    first_child()->DestroyWidget();
  DestroyLayer();
  auto const parent_widget = container_widget();
  parent_widget->RemoveChild(this);
  parent_widget->DidRemoveChildWidget(this);
  if (owned_by_client_)
    return;
  delete this;
}

void Widget::DidHide() {
  visible_ = false;
  container_widget()->DidChangeChildVisibility(this);
  // Hide widgets in top to bottom == post order.
  for (auto child : common::adopters::reverse(child_nodes())) {
    if (!child->is_realized())
      continue;
    child->Hide();
  }
}

void Widget::DidKillFocus(ui::Widget*) {}

void Widget::DidRealize() {
  for (auto const child : child_nodes())
    child->RealizeWidget();
}

void Widget::DidRealizeChildWidget(Widget* widget) {}

void Widget::DidRemoveChildWidget(Widget* widget) {}

void Widget::DidRequestDestroy() {
  ::DestroyWindow(AssociatedHwnd());
}

void Widget::DidChangeBounds() {
  SchedulePaint();
  if (!layer())
    return;
  layer()->SetBounds(bounds());
}

void Widget::DidSetFocus(ui::Widget*) {
  ::SetForegroundWindow(AssociatedHwnd());
}

void Widget::DidShow() {
  visible_ = true;
  container_widget()->DidChangeChildVisibility(this);
  // Show child in bottom to top == pre-order.
  for (auto child : child_nodes()) {
    if (!child->is_realized())
      continue;
    child->Show();
  }
}

void Widget::DispatchMouseExited() {
  auto const hover = hover_widget;
  if (!hover)
    return;
  hover_widget = nullptr;
  auto const screen_point = GetCursorPoint();
  auto const client_point = MapFromDesktopPoint(screen_point);
  MouseEvent event(EventType::MouseExited, MouseButton::None, 0, 0, hover,
                   client_point, screen_point);
  hover->OnMouseExited(event);
}

void Widget::DispatchPaintMessage() {
  RECT raw_exposed_rect;
  if (!::GetUpdateRect(*native_window(), &raw_exposed_rect, false))
    return;
  Rect exposed_rect(raw_exposed_rect);
  OnPaint(exposed_rect);
  for (auto child : child_nodes()) {
    if (!child->visible() || child->has_native_window())
      continue;
    auto const rect = exposed_rect.Intersect(child->bounds());
    if (rect.empty())
      continue;
    child->OnPaint(rect);
  }
}

gfx::RectF Widget::GetContentsBounds() const {
  return gfx::RectF(gfx::SizeF(bounds_.width(), bounds_.height()));
}

HCURSOR Widget::GetCursorAt(const gfx::Point&) const {
  return ::LoadCursor(nullptr, IDC_ARROW);
}

Widget* Widget::GetHostWidget() const {
  for (auto runner : common::tree::ancestors_or_self(this)) {
    if (runner->native_window())
      return const_cast<Widget*>(runner);
  }
  return RootWidget::instance();
}

gfx::Rect Widget::GetLocalBounds() const {
  return gfx::Rect(bounds_.size());
}

gfx::Size Widget::GetPreferredSize() const {
  return gfx::Size();
}

// Note: WM_KEYDOWN events is dispatch maximum 30 times per second. This rate
// is specified by |SPI_GETKEYBOARDSPEED| of |SystemParametersInfo()|, minimum
// is 2.5 repetitions per second through maximum 30 repetitions.
LRESULT Widget::HandleKeyboardMessage(uint32_t message,
                                      WPARAM wParam,
                                      LPARAM lParam) {
  if (message == WM_CHAR) {
    KeyEvent event(EventType::KeyPressed, static_cast<int>(wParam),
                   KeyEvent::ConvertToRepeat(lParam));
    auto const key_code = event.raw_key_code();
    if (key_code < 0x20)
      return 0;
    TRACE_EVENT_ASYNC_BEGIN2("input", "KeyEvent", event.id(), "type",
                             event.type_name(), "key_code",
                             event.raw_key_code());
    OnEvent(&event);
    return 0;
  }

  KeyEvent event(KeyEvent::ConvertToEventType(message),
                 KeyEvent::ConvertToKeyCode(wParam),
                 KeyEvent::ConvertToRepeat(lParam));

  TRACE_EVENT_ASYNC_BEGIN2("input", "KeyEvent", event.id(), "type",
                           event.type_name(), "key_code",
                           event.raw_key_code());

  if (message == WM_KEYDOWN && event.key_code() <= 0x7E && !event.alt_key() &&
      !event.control_key()) {
    // We use WM_CHAR for graphic key down.
    return OnMessage(message, wParam, lParam);
  }

  if (event.type() != EventType::Invalid) {
    OnEvent(&event);
    return 0;
  }
  return OnMessage(message, wParam, lParam);
}

static bool DispatchMouseEvent(Widget* widget, MouseEvent* event) {
  TRACE_EVENT_ASYNC_BEGIN1("input", "MouseEvent", event->id(), "type",
                           event->type_name());
  TRACE_EVENT_WITH_FLOW1("input", "DispatchMouseEvent", event->id(),
                         TRACE_EVENT_FLAG_FLOW_OUT, "type",
                         event->type_name());
  return widget->DispatchEvent(event);
}

bool Widget::HandleMouseMessage(const base::NativeEvent& native_event) {
  auto const client_point = GetClientPointFromNativeEvent(native_event);
  auto const screen_point = GetScreenPointFromNativeEvent(native_event);
  auto const message = native_event.message;
  if (message == WM_MOUSEWHEEL) {
    // Note: We send WM_MOUSEWHEEL message to a widget under mouse pointer
    // rather than active widget.
    auto const result = HitTestForMouseEventTarget(client_point);
    if (!result)
      return true;
    MouseWheelEvent event(result.widget(), result.local_point(), screen_point,
                          MouseEvent::ConvertToEventFlags(native_event),
                          GET_WHEEL_DELTA_WPARAM(native_event.wParam));
    TRACE_EVENT_ASYNC_BEGIN0("input", "WheelEvent", event.id());
    TRACE_EVENT_WITH_FLOW0("input", "WheelEvent", event.id(),
                           TRACE_EVENT_FLAG_FLOW_OUT);
    return result.widget()->DispatchEvent(&event);
  }

  auto const result = HitTestForMouseEventTarget(client_point);
  DCHECK(result);
  if (message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE) {
    if (!hover_widget) {
      TRACKMOUSEEVENT track;
      track.cbSize = sizeof(track);
      track.dwFlags = static_cast<DWORD>(
          message == WM_NCMOUSEMOVE ? TME_NONCLIENT | TME_LEAVE : TME_LEAVE);
      track.hwndTrack = *native_window();
      WIN32_VERIFY(::TrackMouseEvent(&track));
      hover_widget = result.widget();
      MouseEvent event(EventType::MouseEntered, MouseButton::None, 0, 0,
                       hover_widget, result.local_point(), screen_point);
      DispatchMouseEvent(hover_widget, &event);
    } else if (hover_widget != result.widget()) {
      MouseEvent event(EventType::MouseExited, MouseButton::None, 0, 0,
                       hover_widget, result.local_point(), screen_point);
      DispatchMouseEvent(hover_widget, &event);
      hover_widget = result.widget();
      if (hover_widget) {
        MouseEvent entered_event(EventType::MouseEntered, MouseButton::None, 0,
                                 0, hover_widget, result.local_point(),
                                 screen_point);
        DispatchMouseEvent(hover_widget, &entered_event);
      }
    }
  }

  MouseEvent event(native_event, result.widget(), result.local_point(),
                   screen_point);
  if (event.type() == EventType::MouseMoved)
    return DispatchMouseEvent(result.widget(), &event);

  if (event.type() == EventType::MousePressed) {
    MouseClickTracker::instance()->OnMousePressed(event);
    return DispatchMouseEvent(result.widget(), &event);
  }

  if (event.type() == EventType::MouseReleased) {
    MouseClickTracker::instance()->OnMouseReleased(event);
    if (!DispatchMouseEvent(result.widget(), &event))
      return false;
    auto const click_count = MouseClickTracker::instance()->click_count();
    if (!click_count)
      return !event.default_prevented();
    if (event.default_prevented())
      return false;
    MouseEvent click_event(
        EventType::MousePressed, MouseEvent::ConvertToButton(native_event),
        MouseEvent::ConvertToEventFlags(native_event), click_count,
        result.widget(), result.local_point(), screen_point);
    return DispatchMouseEvent(result.widget(), &click_event);
  }

  return true;
}

void Widget::Hide() {
  if (native_window_) {
    ::ShowWindow(*native_window_.get(), SW_HIDE);
    return;
  }
  DidHide();
}

Widget::HitTestResult Widget::HitTest(const gfx::Point& local_point) const {
  if (!GetContentsBounds().Contains(gfx::PointF(local_point)))
    return HitTestResult();

  // On release build by MSVS2013, using reverse() causes AV.
  // for (const auto& child: common::adopters::reverse(child_nodes()))
  for (auto runner = last_child(); runner;
       runner = runner->previous_sibling()) {
    auto const child = runner;
    if (!child->visible())
      continue;
    auto const child_point =
        local_point.Offset(-child->bounds().left(), -child->bounds().top());
    if (auto const result = child->HitTest(child_point))
      return result;
  }
  return HitTestResult(this, local_point);
}

Widget::HitTestResult Widget::HitTestForMouseEventTarget(
    const gfx::Point& host_point) const {
  DCHECK(native_window_);
  if (capture_widget) {
    auto local_point = host_point;
    for (auto runner = capture_widget; runner != this;
         runner = runner->container_widget()) {
      local_point =
          local_point.Offset(-runner->bounds().left(), -runner->bounds().top());
    }
    return HitTestResult(capture_widget, local_point);
  }
  if (auto const result = HitTest(host_point))
    return result;
  return HitTestResult(this, host_point);
}

gfx::Point Widget::MapFromDesktopPoint(const gfx::Point& desktop_point) const {
  POINT hwnd_point(desktop_point);
  auto const hwnd = AssociatedHwnd();
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, hwnd, &hwnd_point, 1));
  auto point = gfx::Point(hwnd_point);
  for (auto runner = this; !runner->has_native_window();
       runner = runner->parent_node()) {
    point = point.Offset(-runner->bounds().left(), -runner->bounds().top());
  }
  return point;
}

gfx::Point Widget::MapToDesktopPoint(const gfx::Point& local_point) const {
  auto point = local_point;
  for (auto runner = this; !runner->has_native_window();
       runner = runner->parent_node()) {
    point = point.Offset(runner->bounds().left(), runner->bounds().top());
  }
  POINT hwnd_point(point);
  auto const hwnd = AssociatedHwnd();
  WIN32_VERIFY(::MapWindowPoints(hwnd, HWND_DESKTOP, &hwnd_point, 1));
  return Point(hwnd_point);
}

void Widget::OnDraw(gfx::Canvas* canvas) {
  for (auto child : child_nodes()) {
    if (!child->visible())
      continue;
    gfx::Canvas::ScopedState child_canvas(canvas);
    canvas->SetOffsetBounds(gfx::RectF(child->bounds()));
    child->OnDraw(canvas);
  }
}

void Widget::OnKeyPressed(const KeyEvent&) {}

void Widget::OnKeyReleased(const KeyEvent&) {}

void Widget::OnMouseEntered(const MouseEvent&) {}

void Widget::OnMouseExited(const MouseEvent&) {}

void Widget::OnMouseMoved(const MouseEvent&) {}

void Widget::OnMousePressed(const MouseEvent&) {}

void Widget::OnMouseReleased(const MouseEvent&) {}

void Widget::OnMouseWheel(const MouseWheelEvent&) {}

LRESULT Widget::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (native_window_)
    return native_window_->DefWindowProc(message, wParam, lParam);
  return container_widget()->OnMessage(message, wParam, lParam);
}

void Widget::OnPaint(const Rect) {}

void Widget::RealizeWidget() {
  DCHECK(!is_realized());
  if (!parent_node()) {
    // Realize top-level widget with native window.
    DCHECK(native_window_);
    state_ = kRealized;
    RootWidget::instance()->AppendChild(this);
    CreateNativeWindow();
    return;
  }

  DCHECK(!bounds_.empty());
  DCHECK(parent_node()->is_realized());
  state_ = kRealized;
  if (native_window_) {
    CreateNativeWindow();
    return;
  }

  DidRealize();
  DidChangeBounds();
  if (parent_node()->visible()) {
    visible_ = true;
    DidShow();
    SchedulePaint();
  }
  container_widget()->DidRealizeChildWidget(this);
}

void Widget::ReleaseCapture() {
  if (capture_widget != this)
    return;
  capture_widget = nullptr;
  ::ReleaseCapture();
}

void Widget::RequestFocus() {
  FocusController::instance()->RequestFocus(this);
}

void Widget::SchedulePaint() {
  DCHECK(is_realized());
  DCHECK(!bounds_.empty());
  SchedulePaintInRect(GetLocalBounds());
}

void Widget::SchedulePaintInRect(const gfx::Rect&) {
  DCHECK(is_realized());
  for (auto runner = this; runner; runner = runner->parent_node()) {
    if (auto const animatable = runner->as<ui::AnimatableWindow>()) {
      animatable->RequestAnimationFrame();
      return;
    }
  }
  if (!parent_node()) {
    // |this| is orphan widget.
    DCHECK(!has_native_window());
    return;
  }
  NOTREACHED();
}

void Widget::SetBounds(const gfx::Point& origin,
                       const gfx::Point& bottom_right) {
  SetBounds(gfx::Rect(origin, bottom_right));
}
void Widget::SetBounds(const gfx::Point& origin, const gfx::Size& size) {
  SetBounds(gfx::Rect(origin, size));
}

void Widget::SetBounds(const gfx::Rect& new_bounds) {
  DCHECK(!new_bounds.empty());
  DCHECK(state_ >= kNotRealized);

  if (bounds_ == new_bounds)
    return;
  if (!is_realized()) {
    bounds_ = new_bounds;
    return;
  }

  if (native_window_) {
    ::SetWindowPos(*native_window_.get(), nullptr, new_bounds.left(),
                   new_bounds.top(), new_bounds.width(), new_bounds.height(),
                   SWP_NOACTIVATE);
    return;
  }

  bounds_ = new_bounds;
  DidChangeBounds();
}

void Widget::SetCapture() {
  // We don't allow nested capture.
  if (capture_widget)
    return;
  ::SetCapture(*GetHostWidget()->native_window());
  capture_widget = this;
}

bool Widget::SetCursor() {
  DCHECK(native_window_);
  auto const cursor_point = GetCursorPoint();
  auto const point = MapFromDesktopPoint(cursor_point);
  auto const result = HitTest(point);
  if (!result)
    return false;
  auto const hCursor = result.widget()->GetCursorAt(result.local_point());
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
    old_parent->WillRemoveChildWidget(this);
    old_parent->RemoveChild(this);
  }
  new_parent->AppendChild(this);
  if (new_parent->is_realized()) {
    if (is_realized()) {
      if (auto const window = native_window())
        ::SetParent(*window, new_parent->AssociatedHwnd());
      DidChangeHierarchy();
    } else {
      RealizeWidget();
    }
  } else {
    DCHECK(!is_realized());
  }
  if (old_parent)
    old_parent->DidRemoveChildWidget(this);
  new_parent->DidAddChildWidget(this);
}

void Widget::Show() {
  if (visible_)
    return;

  if (native_window_) {
    ::ShowWindow(*native_window_.get(), SW_SHOW);
    return;
  }

  DidShow();
  SchedulePaint();
}

void Widget::UpdateBounds() {
  DCHECK(native_window_);
  if (container_widget() != RootWidget::instance())
    return;
  RECT raw_client_rect;
  ::GetClientRect(*native_window_.get(), &raw_client_rect);
  bounds_ = Rect(raw_client_rect);
}

void Widget::WillDestroyWidget() {
  DCHECK_EQ(kBeingDestroyed, state_);
  auto const parent_widget = container_widget();
  parent_widget->WillRemoveChildWidget(this);
  if (hover_widget == this)
    hover_widget = nullptr;
  if (capture_widget == this)
    ReleaseCapture();
  FocusController::instance()->WillDestroyWidget(this);
}

void Widget::WillDestroyNativeWindow() {
  std::vector<Widget*> non_native_children;
  for (auto const child : child_nodes()) {
    DCHECK_NE(child->state_, kBeingDestroyed);
    if (!child->native_window())
      non_native_children.push_back(child);
  }
  for (auto const child : non_native_children)
    child->DestroyWidget();
  WillDestroyWidget();
}

void Widget::WillRemoveChildWidget(Widget* widget) {}

LRESULT Widget::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  DCHECK(native_window_);
  switch (message) {
    case WM_CAPTURECHANGED: {
      auto const new_capture = reinterpret_cast<HWND>(lParam);
      if (capture_widget && capture_widget->AssociatedHwnd() != new_capture)
        capture_widget = nullptr;
      return 0;
    }

    case WM_CLOSE:
      DidRequestDestroy();
      return 0;

    case WM_CREATE: {
      auto const create_data = reinterpret_cast<const CREATESTRUCT*>(lParam);
      if (create_data->hwndParent) {
        bounds_ = gfx::Rect(gfx::Point(create_data->x, create_data->y),
                            gfx::Size(create_data->cx, create_data->cy));
      } else {
        UpdateBounds();
      }
      DidRealize();
      return 0;
    }

    case WM_DESTROY:
      WillDestroyNativeWindow();
      return 0;

    case WM_KILLFOCUS:
      FocusController::instance()->DidKillNativeFocus(this);
      return 0;

    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
      DispatchMouseExited();
      return 0;

    case WM_NCACTIVATE:
      // To handle activating window after blocked |SetForegroundWindow()|
      if (LOWORD(wParam) == WA_ACTIVE)
        FocusController::instance()->DidActivate(this);
      // Ask Windows to process |WM_NCACTIVATE| too.
      return 1;

    case WM_NCDESTROY:
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

    case WM_SETFOCUS: {
      FocusController::instance()->DidSetNativeFocus(this);
      return 0;
    }

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

    case WM_SHOWWINDOW:
      if (wParam)
        DidShow();
      else
        DidHide();
      break;

    case WM_SIZE:
      if (wParam != SIZE_MAXHIDE && wParam != SIZE_MINIMIZED) {
        UpdateBounds();
        if (!bounds_.empty())
          DidChangeBounds();
      }
      return 0;

    case WM_SYSCOLORCHANGE:
      SystemMetrics::instance()->NotifyChangeSystemColor();
      return 0;

    case WM_SYSCOMMAND:
      if ((wParam & 0xFFF0) == SC_KEYMENU) {
        // TODO(eval1749): When we support menu, we should redirect |SC_KEYMENU|
        // to menu for menu shortcut.
        // Note: We use Alt+Key for |accesskey|.
        return 0;
      }
      break;

    case WM_WINDOWPOSCHANGED: {
// DefWindowProc sents WM_SIZE and WM_MOVE, so handling
// WM_WINDPOSCHANGED is faster than DefWindowProc.
// undocumented SWP flags. See http://www.winehq.org.
#if !defined(SWP_NOCLIENTSIZE)
#define SWP_NOCLIENTSIZE 0x0800
#define SWP_NOCLIENTMOVE 0x1000
#endif  // !defined(SWP_NOCLIENTSIZE)
      // Create   0x10001843  NOCLIENTMOVE NOCLIENTSIZE SHOWWINDOW NOMOV NOSIZE
      // Minimize 0x00008130
      // Restore  0x00008124
      // Move     0x00000A15
      // Destroy  0x20001897  NOCLIENTMOVE NOCLIENTSIZE HIDEWINDOW NOACTIVATE
      //                      NOZORDER NOMOVE NOSIZE
      // if (wp->flags & SWP_NOSIZE) return 0;

      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
      if (wp->flags & SWP_NOSIZE)
        return 0;

      if (::IsIconic(*native_window_.get())) {
        // We don't care minimize window.
        return 0;
      }

      if (container_widget() == RootWidget::instance()) {
        UpdateBounds();
      } else {
        bounds_ =
            gfx::Rect(gfx::Point(wp->x, wp->y), gfx::Size(wp->cx, wp->cy));
      }
      DidChangeBounds();
      return 0;
    }
  }

  if (auto const focus_widget = FocusController::instance()->focus_widget()) {
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
      return focus_widget->HandleKeyboardMessage(message, wParam, lParam);
  }

  if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
      (message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK)) {
    base::NativeEvent native_event = {
        AssociatedHwnd(),
        message,
        wParam,
        lParam,
        0,
        {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}};
    if (HandleMouseMessage(native_event))
      return OnMessage(message, wParam, lParam);
    return 0;
  }

  if (message >= WM_IME_STARTCOMPOSITION && message <= WM_IME_KEYLAST) {
    auto result =
        TextInputClientWin::instance()->OnImeMessage(message, wParam, lParam);
    if (result.second)
      return result.first;
    return OnMessage(message, wParam, lParam);
  }

  return OnMessage(message, wParam, lParam);
}

// EventTarget
void Widget::OnKeyEvent(KeyEvent* event) {
  if (event->type() == EventType::KeyPressed) {
    OnKeyPressed(*event);
    return;
  }
  if (event->type() == EventType::KeyReleased) {
    OnKeyReleased(*event);
    return;
  }
  NOTREACHED();
}

void Widget::OnMouseEvent(MouseEvent* event) {
  if (event->type() == EventType::MouseEntered) {
    OnMouseEntered(*event);
    return;
  }
  if (event->type() == EventType::MouseExited) {
    OnMouseExited(*event);
    return;
  }
  if (event->type() == EventType::MouseMoved) {
    OnMouseMoved(*event);
    return;
  }
  if (event->type() == EventType::MousePressed) {
    OnMousePressed(*event);
    return;
  }
  if (event->type() == EventType::MouseReleased) {
    OnMouseReleased(*event);
    return;
  }
  if (event->type() == EventType::MouseWheel) {
    OnMouseWheel(*(event->as<MouseWheelEvent>()));
    return;
  }
  NOTREACHED();
}

}  // namespace ui

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget) {
  out << "{" << widget.class_name() << "@" << std::hex
      << reinterpret_cast<uintptr_t>(&widget) << " " << std::dec
      << widget.bounds() << "}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const ui::Widget* widget) {
  if (widget)
    return out << *widget;
  return out << "null";
}
