// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/window.h"

#include <unordered_map>

#include "evita/base/tree/child_nodes.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/dom/public/view_events.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/events/event.h"
#include "evita/views/tabs/tab_data_set.h"
#include "evita/views/window_set.h"

namespace views {

namespace {

//////////////////////////////////////////////////////////////////////
//
// WindowIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
class WindowIdMapper final : public common::Singleton<WindowIdMapper> {
 public:
  ~WindowIdMapper() = default;

  void DidDestroyDomWindow(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      // The window is destroyed by UI.
      return;
    }
    map_.erase(it);
  }

  Window* Find(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    return it == map_.end() ? nullptr : it->second;
  }

  WindowId Register(Window* window) {
    ASSERT_CALLED_ON_UI_THREAD();
    auto const window_id = window->window_id();
    DCHECK_NE(kInvalidWindowId, window_id);
    DCHECK_EQ(0u, map_.count(window_id));
    map_[window_id] = window;
    return window_id;
  }

  void Unregister(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      // The window is destroyed by DOM.
      return;
    }
    map_.erase(it);
  }

 private:
  friend class common::Singleton<WindowIdMapper>;
  typedef WindowId WindowId;

  WindowIdMapper() = default;

  std::unordered_map<WindowId, Window*> map_;

  DISALLOW_COPY_AND_ASSIGN(WindowIdMapper);
};

domapi::ViewEventHandler* view_event_handler() {
  return editor::Application::instance()->view_event_handler();
}

domapi::EventTargetId MaybeEventTarget(ui::Widget* widget) {
  if (!widget)
    return domapi::kInvalidEventTargetId;
  auto const window = widget->as<Window>();
  return window ? window->window_id() : domapi::kInvalidEventTargetId;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window
//
Window::Window(std::unique_ptr<NativeWindow> native_window, WindowId window_id)
    : ui::Widget(std::move(native_window)),
      EventSource(window_id),
      active_tick_(0),
      window_id_(window_id) {
  DCHECK_NE(views::kInvalidWindowId, window_id_);
  WindowIdMapper::instance()->Register(this);
}

Window::Window(WindowId window_id)
    : Window(std::unique_ptr<NativeWindow>(), window_id) {}

Window::~Window() {
  WindowIdMapper::instance()->Unregister(window_id_);
  view_event_handler()->DidDestroyWindow(window_id_);
}

Window* Window::FromWindowId(WindowId window_id) {
  return WindowIdMapper::instance()->Find(window_id);
}

void Window::DidDestroyDomWindow() {
  TabDataSet::instance()->RemoveTabData(window_id_);
  WindowIdMapper::instance()->DidDestroyDomWindow(window_id_);
}

// ui::Widget
void Window::DidActivate() {
  view_event_handler()->DidActivateWindow(window_id_);
}

void Window::DidChangeBounds() {
  ui::Widget::DidChangeBounds();
  view_event_handler()->DidChangeWindowBounds(window_id_, bounds().left(),
                                              bounds().top(), bounds().right(),
                                              bounds().bottom());
}

void Window::DidHide() {
  ui::Widget::DidHide();
  view_event_handler()->DidChangeWindowVisibility(window_id_,
                                                  domapi::Visibility::Hidden);
}

void Window::DidKillFocus(ui::Widget* will_focus_window) {
  ui::Widget::DidKillFocus(will_focus_window);
  DispatchFocusEvent(domapi::EventType::Blur,
                     MaybeEventTarget(will_focus_window));
}

void Window::DidRealize() {
  view_event_handler()->DidRealizeWidget(window_id_);
  view_event_handler()->DidChangeWindowBounds(window_id_, bounds().left(),
                                              bounds().top(), bounds().right(),
                                              bounds().bottom());
  ui::Widget::DidRealize();
}

void Window::DidSetFocus(ui::Widget* last_focused) {
  CR_DEFINE_STATIC_LOCAL(int, static_active_tick, (0));
  ++static_active_tick;
  active_tick_ = static_active_tick;
  ui::Widget::DidSetFocus(last_focused);
  DispatchFocusEvent(domapi::EventType::Focus, MaybeEventTarget(last_focused));
}

void Window::DidShow() {
  ui::Widget::DidShow();
  view_event_handler()->DidChangeWindowVisibility(window_id_,
                                                  domapi::Visibility::Visible);
}

void Window::OnKeyPressed(const ui::KeyEvent& event) {
  DispatchKeyboardEvent(event);
}

void Window::OnKeyReleased(const ui::KeyEvent& event) {
  DispatchKeyboardEvent(event);
}

void Window::OnMouseMoved(const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void Window::OnMousePressed(const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void Window::OnMouseReleased(const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void Window::OnMouseWheel(const ui::MouseWheelEvent& event) {
  DispatchWheelEvent(event);
}

void Window::WillDestroyWidget() {
  ui::Widget::WillDestroyWidget();
  active_tick_ = 0;
}

}  // namespace views
