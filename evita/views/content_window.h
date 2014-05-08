// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_content_window_h)
#define INCLUDE_evita_views_content_window_h

#include <memory>

#include "base/strings/string16.h"
#include "evita/views/window.h"

class Frame;

namespace ui {
class MouseEvent;
}

namespace views {

class ContentWindow : public Window {
  DECLARE_CASTABLE_CLASS(ContentWindow, Window);

  protected: explicit ContentWindow(views::WindowId window_id);
  public: virtual ~ContentWindow();

  public: Frame& frame() const;

  // [A]
  public: void Activate();

  // [M]
  public: virtual void MakeSelectionVisible() = 0;

  // [R]
  public: virtual void Redraw() = 0;

  // ui::Widget
  protected: virtual void DidSetFocus(ui::Widget*) override;

  DISALLOW_COPY_AND_ASSIGN(ContentWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_content_window_h)
