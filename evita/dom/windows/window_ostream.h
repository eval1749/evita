// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_WINDOW_OSTREAM_H_
#define EVITA_DOM_WINDOWS_WINDOW_OSTREAM_H_

#include <ostream>

namespace dom {
class Window;
enum class Window::State;
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window);
std::ostream& operator<<(std::ostream& ostream, const dom::Window* window);
std::ostream& operator<<(std::ostream& ostream, dom::Window::State state);

#endif  // EVITA_DOM_WINDOWS_WINDOW_OSTREAM_H_
