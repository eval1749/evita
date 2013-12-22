// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/content/content_window.h"

namespace content {

ContentWindow::ContentWindow(
    std::unique_ptr<base::win::NaitiveWindow>&& naitive_window)
    : CommandWindow_(std::move(naitive_window)),
      active_tick_(0) {
}

ContentWindow::ContentWindow()
    : ContentWindow(std::move(std::unique_ptr<base::win::NaitiveWindow>())) {
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
