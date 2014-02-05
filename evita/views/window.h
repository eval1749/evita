// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_window_h)
#define INCLUDE_evita_views_window_h

#include "evita/ui/widget.h"
#include "evita/views/window_id.h"

namespace ui {
class MouseEvent;
}

namespace views {

typedef common::win::NativeWindow NativeWindow;
typedef common::win::Point Point;
typedef common::win::Rect Rect;
class WindowSet;

class Window : public ui::Widget {
  DECLARE_CASTABLE_CLASS(Window, Widget);

  private: const views::WindowId window_id_;

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
  protected: virtual void DidResize() override;
  protected: virtual void DidSetFocus() override;
  protected: void DispatchKeyboardEvent(const ui::KeyboardEvent& event);
  protected: void DispatchMouseEvent(const ui::MouseEvent& event);

  // [F]
  public: static Window* FromWindowId(WindowId window_id);

  // ui::Widget
  protected: virtual void OnKeyPressed(
      const ui::KeyboardEvent& event) override;
  protected: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  protected: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  protected: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_window_h)
