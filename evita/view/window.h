// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_view_window_h)
#define INCLUDE_view_window_h

#include "evita/view/window_id.h"
#include "evita/widgets/widget.h"

namespace view {

typedef common::win::NativeWindow NativeWindow;
typedef common::win::Point Point;
typedef common::win::Rect Rect;

class Window : public widgets::Widget {
  DECLARE_CASTABLE_CLASS(Window, Widget);

  protected: explicit Window(
      std::unique_ptr<NativeWindow>&& native_window,
      WindowId window_id = kInvalidWindowId);
  protected: Window(WindowId window_id = kInvalidWindowId);
  protected: ~Window();
};

}  // namespace view

#endif //!defined(INCLUDE_view_window_h)
