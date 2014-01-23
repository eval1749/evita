// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/window.h"

#include <unordered_map>

#include "evita/editor/application.h"
#include "evita/dom/view_event_handler.h"
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

  public: WindowSet all_windows() const {
    WindowSet::Set windows;
    for (const auto& entry : map_) {
      windows.insert(entry.second);
    }
    return WindowSet(std::move(windows));
  }

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

dom::ViewEventHandler* view_event_handler() {
  return Application::instance()->view_event_handler();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window
//

Window::Window(std::unique_ptr<NativeWindow>&& native_window,
               WindowId window_id)
    : Widget(std::move(native_window)),
      window_id_(window_id) {
  if (window_id != views::kInvalidWindowId)
    WindowIdMapper::instance()->Register(this);
}

Window::Window(WindowId window_id)
    : window_id_(window_id) {
  if (window_id != views::kInvalidWindowId)
    WindowIdMapper::instance()->Register(this);
}

Window::~Window() {
  if (window_id_ != views::kInvalidWindowId) {
    WindowIdMapper::instance()->Unregister(window_id_);
    view_event_handler()->DidDestroyWidget(window_id_);
  }
}

WindowSet Window::all_windows() {
  return WindowIdMapper::instance()->all_windows();
}

void Window::DidDestroyDomWindow() {
  WindowIdMapper::instance()->DidDestroyDomWindow(window_id_);
}

void Window::DidKillFocus() {
  Widget::DidKillFocus();
  if (window_id_ != views::kInvalidWindowId)
    view_event_handler()->DidKillFocus(window_id_);
}

void Window::DidRealize() {
  // TODO(yosi) Until we manage all widgets by WindowId, we don't call
  // ViewEventHandler for unmanaged widget.
  if (window_id_ != views::kInvalidWindowId) {
    view_event_handler()->
        DidRealizeWidget(window_id_);
  }
  Widget::DidRealize();
}

void Window::DidSetFocus() {
  Widget::DidSetFocus();
  if (window_id_ != views::kInvalidWindowId)
    view_event_handler()->DidSetFocus(window_id_);
}

Window* Window::FromWindowId(WindowId window_id) {
  return WindowIdMapper::instance()->Find(window_id);
}

}  // namespace views
