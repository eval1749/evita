#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "widgets/container_widget.h"

#include "common/adoptors/reverse.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "common/tree/descendants_or_self.h"
#include "common/win/native_window.h"
#include "evita/widgets/root_widget.h"
#include <algorithm>

#define DEBUG_FOCUS 0
#define DEBUG_IDLE 0
#define DEBUG_MOUSE 0
#define DEBUG_PAINT 0

namespace widgets {

ContainerWidget::ContainerWidget(
    std::unique_ptr<NativeWindow>&& native_window)
    : ContainerNode_(std::move(native_window)),
      capture_widget_(nullptr),
      focus_widget_(nullptr) {
}

ContainerWidget::ContainerWidget()
    : ContainerWidget(NativeWindow::Create()) {
}

ContainerWidget::~ContainerWidget() {
}

void ContainerWidget::DidAddChildWidget(const Widget&) {
}

void ContainerWidget::DidChangeHierarchy() {
  for (auto& child: child_nodes()) {
    child.DidChangeHierarchy();
  }
}

void ContainerWidget::DidHide() {
  for (auto& child: child_nodes()) {
    child.Hide();
  }
}

void ContainerWidget::DidRealizeChildWidget(const Widget&) {
}

void ContainerWidget::DidRemoveChildWidget(const Widget&) {
}

void ContainerWidget::DidShow() {
  for (auto& child: child_nodes()) {
    child.Show();
  }
}

void ContainerWidget::DispatchPaintMessage() {
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

   for (auto& child : child_nodes()) {
    if (!child.is_shown() || child.has_native_window())
      continue;
    auto const rect = exposed_rect.Intersect(child.rect());
    if (rect) {
      #if DEBUG_PAINT
        DEBUG_WIDGET_PRINTF("Start " DEBUG_WIDGET_FORMAT " focus=%d "
                            DEBUG_RECT_FORMAT "\n",
            DEBUG_WIDGET_ARG(&child),
            child.has_focus(),
            DEBUG_RECT_ARG(rect));
      #endif
      child.OnPaint(rect);
      #if DEBUG_PAINT
        DEBUG_WIDGET_PRINTF("End " DEBUG_RECT_FORMAT "\n",
            DEBUG_RECT_ARG(rect));
      #endif
    }
  }
}

ContainerWidget& ContainerWidget::GetHostContainer() const {
  for (auto& runner: common::tree::ancestors_or_self(*this)) {
    if (runner.native_window())
       return const_cast<ContainerWidget&>(runner);
  }
  return RootWidget::instance();
}

Widget* ContainerWidget::GetWidgetAt(const Point& point) const {
  // On release build by MSVS2013, using reverse() causes AV.
  // for (const auto& child: common::adoptors::reverse(child_nodes()))
  for (auto runner = last_child(); runner;
       runner = runner->previous_sibling()) {
    const auto& child = *runner;
    if (!child.is_shown())
      continue;
    if (child.rect().Contains(point)) {
      if (!child.is_container())
        return const_cast<Widget*>(&child);
      auto child_child = child.ToContainer()->GetWidgetAt(point);
      return child_child ? child_child : const_cast<Widget*>(&child);
    }
  }
  return nullptr;
}

// Hide widgets in post order.
void ContainerWidget::Hide() {
  for (auto& child: common::adoptors::reverse(child_nodes())) {
   child.Hide();
  }
  Widget::Hide();
}

bool ContainerWidget::OnIdle(uint idle_count) {
  #if DEBUG_IDLE
    DEBUG_WIDGET_PRINTF("count=%d\n", idle_count);
  #endif
  auto more = false;
  for (auto& child : child_nodes()) {
    if (child.OnIdle(idle_count))
      more = true;
  }
  return more;
}

void ContainerWidget::ReleaseCaptureFrom(const Widget& widget) {
  auto& host = GetHostContainer();
  #if DEBUG_MOUSE
    DEBUG_WIDGET_PRINTF("capture=" DEBUG_WIDGET_FORMAT " new="
                        DEBUG_WIDGET_FORMAT "\n",
        DEBUG_WIDGET_ARG(host.capture_widget_), DEBUG_WIDGET_ARG(&widget));
  #endif
  ASSERT(widget == host.capture_widget_);
  host.capture_widget_ = nullptr;
  ::ReleaseCapture();
}

void ContainerWidget::SetCaptureTo(const Widget& widget) {
  auto& host = GetHostContainer();
  #if DEBUG_MOUSE
    DEBUG_WIDGET_PRINTF("capture=" DEBUG_WIDGET_FORMAT " new="
                        DEBUG_WIDGET_FORMAT "\n",
        DEBUG_WIDGET_ARG(host.capture_widget_), DEBUG_WIDGET_ARG(&widget));
  #endif
  if (!host.capture_widget_)
    ::SetCapture(*host.native_window());
  host.capture_widget_ = const_cast<Widget*>(&widget);
}

bool ContainerWidget::SetCursor() {
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

void ContainerWidget::SetFocusTo(const Widget& widget) {
  if (!native_window()) {
    GetHostContainer().SetFocusTo(widget);
    return;
  }

  auto const hwnd = static_cast<HWND>(*native_window());

  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("native_focus=%d"
                        " new=" DEBUG_WIDGET_FORMAT
                        " cur=" DEBUG_WIDGET_FORMAT "\n",
        ::GetFocus() == hwnd,
        DEBUG_WIDGET_ARG(&widget),
        DEBUG_WIDGET_ARG(focus_widget_));
  #endif

  ASSERT(Contains(widget));

  if (::GetFocus() != hwnd) {
    ASSERT(!focus_widget_);
    focus_widget_ = const_cast<Widget*>(&widget);
    ::SetFocus(hwnd);
    return;
  }

  if (widget == focus_widget_)
    return;

  if (auto previous = focus_widget_) {
    focus_widget_ = nullptr;
    previous->DidKillFocus();
  }
  focus_widget_ = const_cast<Widget*>(&widget);
  focus_widget_->DidSetFocus();
}

// Show widgets in pre order.
void ContainerWidget::Show() {
  Widget::Show();
  for (auto& child : child_nodes()) {
    child.Show();
  }
}

void ContainerWidget::WillDestroyChildWidget(const Widget& widget) {
  auto& host = GetHostContainer();
  if (widget != host.focus_widget_)
    return;
  host.focus_widget_ = nullptr;
  const_cast<Widget&>(widget).DidKillFocus();
}

void ContainerWidget::WillRemoveChildWidget(const Widget&) {
}

LRESULT ContainerWidget::WindowProc(UINT message, WPARAM wParam,
                                   LPARAM lParam) {
  switch(message) {
    case WM_KILLFOCUS:
      #if DEBUG_FOCUS
        DEBUG_WIDGET_PRINTF("WM_KILLFOCUS cur=" DEBUG_WIDGET_FORMAT "\n",
            DEBUG_WIDGET_ARG(focus_widget_));
      #endif
      if (auto widget = focus_widget_) {
        focus_widget_ = nullptr;
        widget->DidKillFocus();
        return 0;
      }
      break;

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
        DEBUG_WIDGET_PRINTF("WM_SETFOCUS cur=" DEBUG_WIDGET_FORMAT "\n",
            DEBUG_WIDGET_ARG(focus_widget_));
      #endif
      if (auto widget = focus_widget_) {
        widget->DidSetFocus();
        return 0;
      }
      break;
  }

  if (focus_widget_) {
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
      return focus_widget_->OnMessage(message, wParam, lParam);
  }

  if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
    if (capture_widget_) {
      capture_widget_->OnMessage(message, wParam, lParam);
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

  return Widget::WindowProc(message, wParam, lParam);
}

} // namespace widgets
