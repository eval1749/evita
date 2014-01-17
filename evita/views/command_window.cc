// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/command_window.h"

#include "evita/cm_CmdProc.h"
#include "evita/vi_Frame.h"

CommandWindow::CommandWindow(
    std::unique_ptr<widgets::NativeWindow>&& native_window)
    : views::Window(std::move(native_window)) {
}

CommandWindow::CommandWindow(views::WindowId window_id)
    : views::Window(window_id) {
}

CommandWindow::~CommandWindow() {
}

Command::KeyBindEntry* CommandWindow::MapKey(uint key_code) {
  return Command::g_pGlobalBinds->MapKey(key_code);
}

// widgets::Widget
void CommandWindow::DidSetFocus() {
  if (is_shown())
    return;
  auto const frame = Frame::FindFrame(*this);
  frame->DidSetFocusOnChild(this);
}
