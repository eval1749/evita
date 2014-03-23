// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/command_window.h"

#include "evita/vi_Frame.h"

CommandWindow::CommandWindow(views::WindowId window_id)
    : views::Window(window_id) {
}

CommandWindow::~CommandWindow() {
}

// ui::Widget
void CommandWindow::DidSetFocus() {
  Window::DidSetFocus();
  auto const frame = Frame::FindFrame(*this);
  frame->DidSetFocusOnChild(this);
}
