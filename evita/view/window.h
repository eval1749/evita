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
class WindowSet;

class Window : public widgets::Widget {
  DECLARE_CASTABLE_CLASS(Window, Widget);

  private: const view::WindowId window_id_;

  // TODO(yosi): We allow window_id as optional until we export all widgets
  // to DOM.
  protected: Window(
      std::unique_ptr<NativeWindow>&& native_window,
      WindowId window_id = kInvalidWindowId);
  protected: explicit Window(WindowId window_id = kInvalidWindowId);
  protected: ~Window();

  public: static WindowSet all_windows();
  public: WindowId window_id() const { return window_id_; }

  // [D]
  public: void DidDestroyDomWindow();
  protected: virtual void DidKillFocus() override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidSetFocus() override;

  // [F]
  public: static Window* FromWindowId(WindowId window_id);

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace view

#endif //!defined(INCLUDE_view_window_h)
