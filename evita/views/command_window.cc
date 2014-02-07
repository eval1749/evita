// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/command_window.h"

#include "evita/vi_Frame.h"

CommandWindow::CommandWindow(
    std::unique_ptr<ui::NativeWindow>&& native_window)
    : views::Window(std::move(native_window)) {
}

CommandWindow::CommandWindow(views::WindowId window_id)
    : views::Window(window_id) {
}

CommandWindow::~CommandWindow() {
}

// ui::Widget
void CommandWindow::DidSetFocus() {
  Window::DidSetFocus();
  if (is_shown())
    return;
  auto const frame = Frame::FindFrame(*this);
  frame->DidSetFocusOnChild(this);
}
