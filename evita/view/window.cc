// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/window.h"

namespace view {

Window::Window(std::unique_ptr<NativeWindow>&& native_window,
               WindowId window_id)
    : Widget(std::move(native_window), window_id) {
}

Window::Window(WindowId window_id)
    : Widget(window_id) {
}

Window::~Window() {
}

}  // namespace view
