// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/widget.h"

#include <vector>

#include "common/adopters/reverse.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "common/win/win32_verify.h"
#include "evita/ui/base/ime/text_input_client_win.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/mouse_click_tracker.h"
#include "evita/ui/root_widget.h"
#include "evita/ui/system_metrics.h"

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
Widget* hover_widget;
Widget* will_focus_widget;
bool we_have_active_focus;

Point GetCursorPoint() {
  POINT cursor_point;
  WIN32_VERIFY(::GetCursorPos(&cursor_point));
  return Point(cursor_point);
}

}  // namespace

using ui::EventType;

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
class Widget::HitTestResult {
  private: Point local_point_;
  private: Widget* widget_;

  public: HitTestResult(const Widget* widget, const Point& local_point);
  public: HitTestResult(const HitTestResult& other);
  public: HitTestResult();
  public: ~HitTestResult() = default;

  public: explicit operator bool() const { return widget_; }

  public: const Point& local_point() const { return local_point_; }
  public: Widget* widget() const { return widget_; }
};

Widget::HitTestResult::HitTestResult(const Widget* widget,
                                     const Point& local_point)
    : local_point_(local_point), widget_(const_cast<Widget*>(widget)) {
}

Widget::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.widget_, other.local_point_) {
}

Widget::HitTestResult::HitTestResult()
    : HitTestResult(nullptr, Point()) {
}

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
    DVLOG(0) << "~Widget: " << *this;
  #endif
  DCHECK(!native_window_);
}

// Child window is assigned in |common::NativeWindow::CreateWindowEx()|.
UINT_PTR Widget::child_window_id() const {
  DCHECK(native_window());
  DCHECK(parent_node());
  return reinterpret_cast<UINT_PTR>(native_window());
}

bool Widget::has_active_focus() {
  // When mode less dialog has focus, ::GetFocus() returns it, but
  // we_have_active_focus is false.
  return we_have_active_focus;
}

bool Widget::has_focus() const {
  return focus_widget == this;
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

void Widget::CreateNativeWindow() const {
}

void Widget::DestroyWidget() {
  #if DEBUG_DESTROY
    DVLOG(0) << "DestroyWidget " << *this;
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
  if (hover_widget == this)
    hover_widget = nullptr;
  if (capture_widget == this)
    ReleaseCapture();
  if (focus_widget == this) {
    focus_widget = nullptr;
    DidKillFocus(nullptr);
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

void Widget::DidChangeChildVisibility(Widget*) {
}

void Widget::DidDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DVLOG(0) << "DidDestroyNativeWindow " << *this;
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
  container_widget().DidChangeChildVisibility(this);
}

void Widget::DidKillFocus(ui::Widget*) {
}

void Widget::DidKillNativeFocus() {
  if (auto widget = focus_widget) {
    focus_widget = nullptr;
    widget->DidKillFocus(will_focus_widget);
  }
  we_have_active_focus = false;
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

void Widget::DidChangeBounds() {
  if (layer())
    layer()->SetBounds(gfx::RectF(bounds()));
}

void Widget::DidSetFocus(ui::Widget*) {
}

void Widget::DidSetNativeFocus() {
  auto const last_focused_widget = focus_widget;
  focus_widget = will_focus_widget ? will_focus_widget : this;
  will_focus_widget = nullptr;
  focus_widget->DidSetFocus(last_focused_widget);
  we_have_active_focus = true;
}

void Widget::DidShow() {
  container_widget().DidChangeChildVisibility(this);
  for (auto child : child_nodes()) {
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
  MouseEvent event(EventType::MouseExited, MouseEvent::kNone, 0u, 0,
                   hover, client_point, screen_point);
  hover->OnMouseExited(event);
}

void Widget::DispatchPaintMessage() {
  RECT raw_exposed_rect;
  if (!::GetUpdateRect(*native_window(), &raw_exposed_rect, false))
    return;
  Rect exposed_rect(raw_exposed_rect);
  #if DEBUG_PAINT
    DVLOG(0) << "DispatchPaintMessage " << *this << " " << exposed_rect;
  #endif
  OnPaint(exposed_rect);
  #if DEBUG_PAINT
    DVLOG(0) << "End " << exposed_rect;
  #endif

   for (auto child : child_nodes()) {
    if (!child->visible() || child->has_native_window())
      continue;
    auto const rect = exposed_rect.Intersect(child->bounds());
    if (!rect.empty()) {
      #if DEBUG_PAINT
        DVLOG(0) << "Start " << child << " focus=" << child.has_focus() <<
            " " << rect;
      #endif
      child->OnPaint(rect);
      #if DEBUG_PAINT
        DVLOG(0) << "End " << rect;
      #endif
    }
  }
}

gfx::RectF Widget::GetContentsBounds() const {
  return gfx::RectF(gfx::SizeF(bounds_.width(), bounds_.height()));
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

gfx::Size Widget::GetPreferredSize() const {
  return gfx::Size();
}

LRESULT Widget::HandleKeyboardMessage(uint32_t message, WPARAM wParam,
                                      LPARAM lParam) {
  if (message == WM_CHAR) {
    KeyboardEvent event(EventType::KeyPressed, static_cast<int>(wParam),
                        KeyboardEvent::ConvertToRepeat(lParam));
    if (event.raw_key_code() >= 0x20)
      OnKeyPressed(event);
    return 0;
  }

  KeyboardEvent event(KeyboardEvent::ConvertToEventType(message),
                      KeyboardEvent::ConvertToKeyCode(wParam),
                      KeyboardEvent::ConvertToRepeat(lParam));

  if (message == WM_KEYDOWN && event.key_code() <= 0x7E && !event.alt_key() &&
      !event.control_key()) {
    // We use WM_CHAR for graphic key down.
    return OnMessage(message, wParam, lParam);
  }

  if (event.event_type() == EventType::KeyPressed) {
    OnKeyPressed(event);
    return 0;
  }

  if (event.event_type() == EventType::KeyReleased) {
    OnKeyReleased(event);
    return 0;
  }

  return OnMessage(message, wParam, lParam);
}

void Widget::HandleMouseMessage(uint32_t message, WPARAM wParam,
                                LPARAM lParam) {
  if (message == WM_MOUSEWHEEL) {
    // Note: We send WM_MOUSEWHEEL message to a widget under mouse pointer
    // rather than active widget.
    Point screen_point(MAKEPOINTS(lParam));
    auto const client_point = MapFromDesktopPoint(screen_point);
    auto const result = HitTestForMouseEventTarget(client_point);
    if (!result)
      return;
    MouseWheelEvent event(result.widget(), result.local_point(), screen_point,
                          GET_KEYSTATE_WPARAM(wParam),
                          GET_WHEEL_DELTA_WPARAM(wParam));
    result.widget()->OnMouseWheel(event);
    return;
  }

  auto const host_point = Point(MAKEPOINTS(lParam));
  auto const screen_point = MapToDesktopPoint(host_point);
  auto const result = HitTestForMouseEventTarget(host_point);
  DCHECK(result);
  if (message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE) {
    if (!hover_widget) {
      TRACKMOUSEEVENT track;
      track.cbSize = sizeof(track);
      track.dwFlags = static_cast<DWORD>(
          message == WM_NCMOUSEMOVE ? TME_NONCLIENT | TME_LEAVE : TME_LEAVE);
      track.hwndTrack = *native_window();
      if (::TrackMouseEvent(&track))
        hover_widget = result.widget();
      else
        DVLOG(0) << "TrackMouseEvent last_error=" << ::GetLastError();
    } else if (hover_widget != result.widget()) {
      MouseEvent event(EventType::MouseExited, MouseEvent::kNone, 0u, 0,
                       hover_widget, result.local_point(), screen_point);
      hover_widget->OnMouseExited(event);
      hover_widget = result.widget();
    }
  }

  MouseEvent event(MouseEvent::ConvertToEventType(message),
                   MouseEvent::ConvertToButton(message, wParam),
                   GET_KEYSTATE_WPARAM(wParam), 0,
                   result.widget(), result.local_point(), screen_point);
  if (event.event_type() == EventType::MouseMoved) {
    result.widget()->OnMouseMoved(event);
    return;
  }

  if (event.event_type() == EventType::MousePressed) {
    MouseClickTracker::instance()->OnMousePressed(event);
    result.widget()->OnMousePressed(event);
    return;
  }

  if (event.event_type() == EventType::MouseReleased) {
    MouseClickTracker::instance()->OnMouseReleased(event);
    result.widget()->OnMouseReleased(event);
    auto const click_count  = MouseClickTracker::instance()->click_count();
    if (!click_count)
      return;
    MouseEvent click_event(EventType::MousePressed,
                           MouseEvent::ConvertToButton(message, wParam),
                           GET_KEYSTATE_WPARAM(wParam), click_count,
                           result.widget(), result.local_point(),
                           screen_point);
    result.widget()->OnMousePressed(click_event);
    return;
  }
}

void Widget::Hide() {
  #if DEBUG_SHOW
    DVLOG_WIDGET(0) << "focus=" << has_focus() << " show=" << shown_;
  #endif
  // Hide widgets in top to bottom == post order.
  for (auto child : common::adopters::reverse(child_nodes())) {
    child->Hide();
  }
  shown_ = 0;
  if (native_window_)
    ::ShowWindow(*native_window_.get(), SW_HIDE);
  else
    DidHide();
}

Widget::HitTestResult Widget::HitTest(const Point& local_point) const {
  if (!GetContentsBounds().Contains(gfx::PointF(local_point)))
    return HitTestResult();

  // On release build by MSVS2013, using reverse() causes AV.
  // for (const auto& child: common::adopters::reverse(child_nodes()))
  for (auto runner = last_child(); runner;
       runner = runner->previous_sibling()) {
    auto const child = runner;
    if (!child->visible())
      continue;
    auto const child_point = local_point.Offset(-child->bounds().left(),
                                                -child->bounds().top());
    if (auto const result = child->HitTest(child_point))
      return result;
  }
  return HitTestResult(this, local_point);
}

Widget::HitTestResult Widget::HitTestForMouseEventTarget(
    const Point& host_point) const {
  DCHECK(native_window_);
  if (capture_widget) {
    auto local_point = host_point;
    for (auto runner = capture_widget; runner != this;
         runner = &runner->container_widget()) {
      local_point = local_point.Offset(-runner->bounds().left(),
                                       -runner->bounds().top());
    }
    return HitTestResult(capture_widget, local_point);
  }
  if (auto const result = HitTest(host_point))
    return result;
  return HitTestResult(this, host_point);
}

Point Widget::MapFromDesktopPoint(const Point& desktop_point) const {
  POINT hwnd_point(desktop_point);
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, AssociatedHwnd(),
                                 &hwnd_point, 1));
  if (*native_window())
    return Point(hwnd_point);
  return Point(hwnd_point.x - bounds().left(), hwnd_point.y - bounds().top());
}

Point Widget::MapToDesktopPoint(const Point& local_point) const {
  POINT point(local_point);
  WIN32_VERIFY(::MapWindowPoints(AssociatedHwnd(), HWND_DESKTOP, &point, 1));
  if (*native_window())
    return Point(point);
  return Point(point.x + bounds().left(), point.y + bounds().top());
}

void Widget::OnDraw(gfx::Canvas* gfx) {
  for (auto child : child_nodes()) {
    if (child->visible())
      child->OnDraw(gfx);
  }
}

void Widget::OnKeyPressed(const KeyboardEvent&) {
}

void Widget::OnKeyReleased(const KeyboardEvent&) {
}

void Widget::OnMouseExited(const MouseEvent&) {
}

void Widget::OnMouseMoved(const MouseEvent&) {
}

void Widget::OnMousePressed(const MouseEvent&) {
}

void Widget::OnMouseReleased(const MouseEvent&) {
}

void Widget::OnMouseWheel(const MouseWheelEvent&) {
}

LRESULT Widget::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (native_window_)
    return native_window_->DefWindowProc(message, wParam, lParam);
  return container_widget().OnMessage(message, wParam, lParam);
}

void Widget::OnPaint(const Rect) {
}

void Widget::OnScroll(int) {
}

void Widget::Realize(const Rect& rect) {
  DCHECK(!is_realized());
  DCHECK(parent_node());
  DCHECK(container_widget().is_realized());

  state_ = kRealized;
  bounds_ = rect;
  if (native_window_) {
    CreateNativeWindow();
    return;
  }

  DidRealize();
  DidChangeBounds();
  if (parent_node()->visible()) {
    shown_ = 1;
    DidShow();
    SchedulePaint();
  }
  container_widget().DidRealizeChildWidget(*this);
}

// TODO(yosi) Widget::RealizeWidget() should be pure virutal.
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

  DCHECK(parent_node()->is_realized());
  state_ = kRealized;
  if (native_window_) {
    CreateNativeWindow();
    return;
  }

  DidRealize();
  DidChangeBounds();
  if (parent_node()->visible()) {
    shown_ = 1;
    DidShow();
    SchedulePaint();
  }
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
      last_focus_widget->DidKillFocus(this);
    focus_widget->DidSetFocus(last_focus_widget);
    return;
  }
  will_focus_widget = this;
  ::SetFocus(*host.native_window());
}

void Widget::SchedulePaint() {
  DCHECK(is_realized());
  DCHECK(!bounds_.empty());
  SchedulePaintInRect(Rect(Point(), bounds_.size()));
}

void Widget::SchedulePaintInRect(const Rect& rect) {
  DCHECK(is_realized());
  DCHECK(!rect.empty());
  // TODO(yosi) We should have |DCHECK(bounds_.Contains(rect))|.
  RECT raw_rect(rect);
  ::InvalidateRect(AssociatedHwnd(), &raw_rect, true);
}

void Widget::SetBounds(const Rect& rect) {
  DCHECK(!rect.empty());
  DCHECK(state_ >= kNotRealized);

#if 0
  // TODO(yosi): We should enable this check.
  if (rect == bounds_)
    return;
#endif
  if (is_realized() && native_window_) {
    ::SetWindowPos(*native_window_.get(), nullptr, rect.left(), rect.top(),
                   rect.width(), rect.height(), SWP_NOACTIVATE);
  } else {
    if (bounds_ > rect && parent_node()) {
      // TODO(yosi) We should schedule paint reveal rectangles only.
      parent_node()->SchedulePaintInRect(bounds_);
    }
    bounds_ = rect;
    DidChangeBounds();
  }
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

void Widget::SetLayer(Layer* layer) {
  DCHECK(!layer_);
  layer_.reset(layer);
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
    old_parent->DidRemoveChildWidget(*this);
  new_parent->DidAddChildWidget(*this);
}

void Widget::Show() {
  #if DEBUG_SHOW
    DVLOG_WIDGET(0) << "focus=" << has_focus() << " show=" << shown_;
  #endif
  ++shown_;
  if (native_window_) {
    ::ShowWindow(*native_window_.get(), SW_SHOW);
  } else if (shown_ == 1) {
    DidShow();
    if (!bounds().empty())
      SchedulePaint();
  }

  // Show child in bottom to top == pre-order.
  for (auto child : child_nodes()) {
    child->Show();
  }
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
  #if DEBUG_DESTROY
    DVLOG(0) << "WillDestroyWidget state=" << state_ << " shown_=" << shown_ <<
        " " << bounds_;
  #endif
}

void Widget::WillDestroyNativeWindow() {
  #if DEBUG_DESTROY
    DVLOG(0) << "WillDestroyNativeWindow state=" << state_ << " shown_=" <<
        shown_ << " " << bounds_;
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
    case WM_CREATE: {
      auto const create_data = reinterpret_cast<const CREATESTRUCT*>(lParam);
      if (create_data->style & WS_VISIBLE)
        ++shown_;
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
      #if DEBUG_FOCUS
        DVLOG_WIDGET(0) << "WM_KILLFOCUS" <<
            " wParam=" << reinterpret_cast<HWND>(wParam) <<
            " cur=" << focus_widget <<
            " will=" << will_focus_widget;
      #endif
      DidKillNativeFocus();
      return 0;

    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
      DispatchMouseExited();
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

    case WM_SETFOCUS: {
      #if DEBUG_FOCUS
        DVLOG_WIDGET(0) << "WM_SETFOCUS" <<
            " wParam=" << reinterpret_cast<HWND>(wParam) <<
            " cur=" << focus_widget <<
            " will=" << will_focus_widget;
      #endif
      DidSetNativeFocus();
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

    case WM_SIZE:
      #if DEBUG_RESIZE
        DVLOG_WIDGET(0) << "WM_SIZE " << wParam << " " <<
            LOWORD(lParam) << "x" << HIWORD(lParam);
      #endif
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
       // TODO(yosi) When we support menu, we should redirect |SC_KEYMENU| to
       // menu for menu shortcut.
       // We use Alt+Key for |accesskey|.
       return 0;
     }
     break;

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

      if (container_widget() == RootWidget::instance()) {
        UpdateBounds();
      } else {
        bounds_ = gfx::Rect(gfx::Point(wp->x, wp->y),
                            gfx::Size(wp->cx, wp->cy));
      }
      DidChangeBounds();
      return 0;
    }
  }

  if (focus_widget) {
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
      return focus_widget->HandleKeyboardMessage(message, wParam, lParam);
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    HandleMouseMessage(message, wParam, lParam);
    return 0;
  }

  if (message >= WM_IME_STARTCOMPOSITION && message<= WM_IME_KEYLAST) {
    auto result = TextInputClientWin::instance()->OnImeMessage(
        message, wParam, lParam);
    if (result.second)
      return result.first;
    return OnMessage(message, wParam, lParam);
  }

  return OnMessage(message, wParam, lParam);
}

} // namespace ui

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget) {
  out << "{" << widget.class_name() << "@" << std::hex <<
    reinterpret_cast<uintptr_t>(&widget) << " " <<
    std::dec << widget.bounds() << "}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const ui::Widget* widget) {
  if (widget)
    return out << *widget;
  return out << "null";
}
