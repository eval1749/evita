// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_ostream_h)
#define INCLUDE_evita_dom_window_ostream_h

namespace dom {
class Window;
enum class Window::State;
}

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window);
std::ostream& operator<<(std::ostream& ostream, const dom::Window* window);
std::ostream& operator<<(std::ostream& ostream, dom::Window::State state);

#endif //!defined(INCLUDE_evita_dom_window_ostream_h)
