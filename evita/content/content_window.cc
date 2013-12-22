// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/content/content_window.h"

#include "evita/vi_Frame.h"

namespace content {

ContentWindow::ContentWindow(
    std::unique_ptr<base::win::NaitiveWindow>&& naitive_window)
    : CommandWindow_(std::move(naitive_window)),
      active_tick_(0) {
}

ContentWindow::ContentWindow()
    : ContentWindow(std::move(std::unique_ptr<base::win::NaitiveWindow>())) {
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

void ContentWindow::DidSetFocus() {
  DEFINE_STATIC_LOCAL(uint, global_active_tick, (0));
  ++global_active_tick;
  active_tick_ = global_active_tick;
}

void ContentWindow::WillDestroyWidget() {
  active_tick_ = 0;
}

}  // namespace content
