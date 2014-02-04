// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/window.h"

#include <unordered_map>

#include "evita/editor/application.h"
#include "evita/dom/public/api_event.h"
#include "evita/dom/view_event_handler.h"
#include "evita/ui/events/event.h"
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

domapi::EventType ConvertEventType(const ui::KeyboardEvent event) {
  auto const event_type = event.event_type();
  if (event_type == ui::EventType::KeyPressed)
    return domapi::EventType::KeyDown;
  if (event_type == ui::EventType::KeyReleased)
    return domapi::EventType::KeyUp;
  return domapi::EventType::Invalid;
}

domapi::EventType ConvertEventType(const ui::MouseEvent event) {
  auto const event_type = event.event_type();
  if (event_type == ui::EventType::MousePressed) {
    if (!event.click_count())
      return domapi::EventType::MouseDown;
    if (event.click_count() == 1)
      return domapi::EventType::Click;
    return domapi::EventType::DblClick;
  }

  if (event_type == ui::EventType::MouseReleased)
    return domapi::EventType::MouseUp;

  if (event_type == ui::EventType::MouseMoved)
    return domapi::EventType::MouseMove;

  if (event_type == ui::EventType::MouseWheel)
    return domapi::EventType::Wheel;

  return domapi::EventType::Invalid;
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

void Window::DispatchKeyboardEvent(const ui::KeyboardEvent& event) {
  domapi::KeyboardEvent api_event;
  api_event.alt_key = false;
  api_event.control_key = event.control_key();
  api_event.event_type = ConvertEventType(event);
  api_event.key_code = event.raw_key_code();
  api_event.location = 0;
  api_event.repeat = event.repeat();
  api_event.meta_key = false;
  api_event.shift_key = event.shift_key();
  api_event.target_id = window_id();
  Application::instance()->view_event_handler()->DispatchKeyboardEvent(
      api_event);
}

void Window::DispatchMouseEvent(const ui::MouseEvent& event) {
  #define MUST_EQUAL(name) \
    static_assert(static_cast<int>(domapi::MouseButton::name) == \
                  ui::MouseEvent::k ## name, \
                  "Button name " # name " must be equal.")
  MUST_EQUAL(Left);
  MUST_EQUAL(Middle);
  MUST_EQUAL(Right);
  MUST_EQUAL(Other1);
  MUST_EQUAL(Other2);

  domapi::MouseEvent api_event;
  api_event.alt_key = event.alt_key();
  api_event.button = static_cast<domapi::MouseButton>(event.button());
  api_event.buttons = event.buttons();
  api_event.client_x = event.location().x;
  api_event.client_y = event.location().y;
  api_event.control_key = event.control_key();
  api_event.event_type = ConvertEventType(event);
  api_event.shift_key = event.shift_key();
  api_event.target_id = window_id();
  Application::instance()->view_event_handler()->DispatchMouseEvent(
      api_event);
}

void Window::OnKeyPressed(const ui::KeyboardEvent& event) {
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

void Window::DidRealize() {
  // TODO(yosi) Until we manage all widgets by WindowId, we don't call
  // ViewEventHandler for unmanaged widget.
  if (window_id_ != views::kInvalidWindowId) {
    view_event_handler()->DidRealizeWidget(window_id_);
    view_event_handler()->DidResizeWidget(window_id_, rect().left, rect().top,
                                          rect().right, rect().bottom);
  }
  Widget::DidRealize();
}

void Window::DidResize() {
  Widget::DidResize();
  view_event_handler()->DidResizeWidget(window_id_, rect().left, rect().top,
                                        rect().right, rect().bottom);
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
