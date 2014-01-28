// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/dom/public/api_event.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/ui/events/event.h"
#include "evita/vi_Frame.h"

namespace views {

namespace {
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

ContentWindow::ContentWindow(
    std::unique_ptr<common::win::NativeWindow>&& native_window)
    : CommandWindow_(std::move(native_window)),
      active_tick_(0) {
}

ContentWindow::ContentWindow(views::WindowId window_id)
    : CommandWindow_(window_id),
      active_tick_(0) {
}

ContentWindow::ContentWindow()
    : ContentWindow(views::kInvalidWindowId) {
}

Frame& ContentWindow::frame() const {
  for (auto runner = static_cast<const Widget*>(this); runner;
       runner = &runner->container_widget()) {
    if (runner->is<Frame>())
      return *const_cast<Frame*>(runner->as<Frame>());
  }
  CAN_NOT_HAPPEN();
}

void ContentWindow::Activate() {
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), is_shown());
  #endif
  SetFocus();
}

void ContentWindow::DidKillFocus() {
  CommandWindow_::DidKillFocus();
}
  
void ContentWindow::DidSetFocus() {
  DEFINE_STATIC_LOCAL(uint, global_active_tick, (0));
  CommandWindow_::DidSetFocus();
  ++global_active_tick;
  active_tick_ = global_active_tick;
}

void ContentWindow::DispatchMouseEvent(const ui::MouseEvent& event) {
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

void ContentWindow::WillDestroyWidget() {
  active_tick_ = 0;
}

}  // namespace views
