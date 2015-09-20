// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_ostream.h"

namespace dom {
namespace internal {
const char* WindowStateString(dom::Window::State state);
}
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window) {
  ostream << "(" << window.wrapper_info()->class_name()
          << " widget:" << window.window_id() << ")";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window* window) {
  return ostream << *window;
}

std::ostream& operator<<(std::ostream& ostream, dom::Window::State state) {
  return ostream << dom::internal::WindowStateString(state);
}
