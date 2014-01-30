// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/command_window.h"

#include "evita/cm_CmdProc.h"
#include "evita/vi_Frame.h"

namespace {
Command::KeyBinds* key_bindings;
}

CommandWindow::CommandWindow(
    std::unique_ptr<ui::NativeWindow>&& native_window)
    : views::Window(std::move(native_window)) {
}

CommandWindow::CommandWindow(views::WindowId window_id)
    : views::Window(window_id) {
}

CommandWindow::~CommandWindow() {
}

void CommandWindow::BindKey(int key_code,
    const common::scoped_refptr<Command::KeyBindEntry>& entry) {
  if (!key_bindings)
    key_bindings = new Command::KeyBinds;
  key_bindings->Bind(key_code, entry);
}

void CommandWindow::BindKey(uint32 key_code,
                            Command::Command::CommandFn function) {
  if (!key_bindings)
    key_bindings = new Command::KeyBinds;
  key_bindings->Bind(static_cast<int>(key_code), function);
}

Command::KeyBindEntry* CommandWindow::MapKey(uint key_code) {
  return key_bindings->MapKey(static_cast<int>(key_code));
}

// ui::Widget
void CommandWindow::DidSetFocus() {
  Window::DidSetFocus();
  if (is_shown())
    return;
  auto const frame = Frame::FindFrame(*this);
  frame->DidSetFocusOnChild(this);
}
