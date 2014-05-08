// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/vi_Frame.h"

namespace views {

ContentWindow::ContentWindow(views::WindowId window_id)
    : Window(window_id) {
}

ContentWindow::~ContentWindow() {
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
  RequestFocus();
}

// ui::Widget
void ContentWindow::DidSetFocus(ui::Widget* widget) {
  Window::DidSetFocus(widget);
  auto const frame = Frame::FindFrame(*this);
  frame->DidSetFocusOnChild(this);
}

}  // namespace views
