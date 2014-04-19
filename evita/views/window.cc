// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/window.h"

#include <unordered_map>

#include "common/tree/child_nodes.h"
#include "evita/editor/application.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/ui/events/event.h"
#include "evita/views/tab_data_set.h"
#include "evita/views/window_set.h"

namespace views {

namespace {

//////////////////////////////////////////////////////////////////////
//
// WindowIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
class WindowIdMapper : public common::Singleton<WindowIdMapper> {
  friend class common::Singleton<WindowIdMapper>;

  private: typedef WindowId WindowId;

  private: std::unordered_map<WindowId, Window*> map_;

  private: WindowIdMapper() = default;
  public: ~WindowIdMapper() = default;

  public: void DidDestroyDomWindow(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      // The window is destroyed by UI.
      return;
    }
    map_.erase(it);
  }

  public: Window* Find(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    return it == map_.end() ? nullptr : it->second;
  }

  public: WindowId Register(Window* window) {
    ASSERT_CALLED_ON_UI_THREAD();
    auto const window_id = window->window_id();
    DCHECK_NE(kInvalidWindowId, window_id);
    DCHECK_EQ(0u, map_.count(window_id));
    map_[window_id] = window;
    return window_id;
  }

  public: void Unregister(WindowId window_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      // The window is destroyed by DOM.
      return;
    }
    map_.erase(it);
  }
};

domapi::ViewEventHandler* view_event_handler() {
  return Application::instance()->view_event_handler();
}

domapi::EventTargetId MaybeEventTarget(ui::Widget* widget) {
  if (!widget)
    return domapi::kInvalidEventTargetId;
  auto const window = widget->as<Window>();
  return window ? window->window_id() : domapi::kInvalidEventTargetId;
}

int static_active_tick;

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window
//

Window::Window(std::unique_ptr<NativeWindow>&& native_window,
               WindowId window_id)
    : Widget(std::move(native_window)),
      EventSource(window_id),
      active_tick_(0),
      window_id_(window_id) {
  DCHECK_NE(views::kInvalidWindowId, window_id_);
  WindowIdMapper::instance()->Register(this);
}

Window::Window(WindowId window_id)
    : Window(std::unique_ptr<NativeWindow>(), window_id) {
}

Window::~Window() {
  WindowIdMapper::instance()->Unregister(window_id_);
  view_event_handler()->DidDestroyWidget(window_id_);
}

void Window::DidDestroyDomWindow() {
  TabDataSet::instance()->RemoveTabData(window_id_);
  WindowIdMapper::instance()->DidDestroyDomWindow(window_id_);
}

void Window::DidKillFocus(ui::Widget* focused_window) {
  Widget::DidKillFocus(focused_window);
  DispatchFocusEvent(domapi::EventType::Blur, MaybeEventTarget(focused_window));
}

bool Window::OnIdle(int hint) {
  auto more = false;
  for (auto child : child_nodes()) {
    if (auto const window = child->as<Window>()) {
      if (window->OnIdle(hint))
        more = true;
    }
  }
  return more;
}

// ui::Wiget
void Window::OnKeyPressed(const ui::KeyboardEvent& event) {
  DispatchKeyboardEvent(event);
}

void Window::OnKeyReleased(const ui::KeyboardEvent& event) {
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

void Window::DidRealize() {
  view_event_handler()->DidRealizeWidget(window_id_);
  view_event_handler()->DidResizeWidget(window_id_, bounds().left, bounds().top,
                                        bounds().right, bounds().bottom);
  Widget::DidRealize();
}

void Window::DidResize() {
  Widget::DidResize();
  view_event_handler()->DidResizeWidget(window_id_, bounds().left, bounds().top,
                                        bounds().right, bounds().bottom);
}

void Window::DidSetFocus(ui::Widget* last_focused) {
  DEFINE_STATIC_LOCAL(int, static_active_tick, (0));
  ++static_active_tick;
  active_tick_ = static_active_tick;
  Widget::DidSetFocus(last_focused);
  DispatchFocusEvent(domapi::EventType::Focus, MaybeEventTarget(last_focused));
}

Window* Window::FromWindowId(WindowId window_id) {
  return WindowIdMapper::instance()->Find(window_id);
}

void Window::WillDestroyWidget() {
  Widget::WillDestroyWidget();
  active_tick_ = 0;
}
}  // namespace views
