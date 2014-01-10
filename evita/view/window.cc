// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/window.h"

#include <unordered_map>

#include "evita/editor/application.h"
#include "evita/dom/view_event_handler.h"

namespace view {

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
      DVLOG(0) << "Why we don't have a window for WindowId " << window_id <<
        " in WindowIdMap?";
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
    map_[window_id] = nullptr;
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
  if (window_id != view::kInvalidWindowId)
    WindowIdMapper::instance()->Register(this);
}

Window::Window(WindowId window_id)
    : window_id_(window_id) {
}

Window::~Window() {
  if (window_id_ != view::kInvalidWindowId) {
    WindowIdMapper::instance()->Unregister(window_id_);
    view_event_handler()->DidDestroyWidget(window_id_);
  }
}

void Window::DidDestroyDomWindow() {
  WindowIdMapper::instance()->DidDestroyDomWindow(window_id_);
}

void Window::DidKillFocus() {
  Widget::DidKillFocus();
  if (window_id_ != view::kInvalidWindowId)
    view_event_handler()->DidKillFocus(window_id_);
}

void Window::DidRealize() {
  // TODO(yosi) Until we manage all widgets by WindowId, we don't call
  // ViewEventHandler for unmanaged widget.
  if (window_id_ != view::kInvalidWindowId) {
    view_event_handler()->
        DidRealizeWidget(window_id_);
  }
  Widget::DidRealize();
}

void Window::DidSetFocus() {
  Widget::DidSetFocus();
  if (window_id_ != view::kInvalidWindowId)
    view_event_handler()->DidSetFocus(window_id_);
}

Window* Window::FromWindowId(WindowId window_id) {
  return WindowIdMapper::instance()->Find(window_id);
}

}  // namespace view
