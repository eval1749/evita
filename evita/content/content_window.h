// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_content_content_window_h)
#define INCLUDE_evita_content_content_window_h

#include <memory>

#include "base/strings/string16.h"
#include "evita/vi_CommandWindow.h"
#include "evita/view/window_id.h"

class Frame;

namespace content {

class ContentWindow : public CommandWindow_<ContentWindow> {
  DECLARE_CASTABLE_CLASS(ContentWindow, CommandWindow);

  private: uint active_tick_;

  protected: explicit ContentWindow(
                std::unique_ptr<common::win::NativeWindow>&&);
  protected: explicit ContentWindow(view::WindowId window_id);
  protected: ContentWindow();

  public: Frame& frame() const;

  // [A]
  public: void Activate();

  // [D]
  protected: virtual void DidKillFocus() override;
  protected: virtual void DidSetFocus() override;

  // [G]
  public: uint GetActiveTick() const { return active_tick_; }
  public: static const char* GetClass_() { return "ContentWindow"; }
  public: virtual base::string16 GetTitle(size_t max_length) const = 0;

  // [M]
  public: virtual void MakeSelectionVisible() = 0;

  // [R]
  public: virtual void Redraw() = 0;

  // [U]
  // TODO(yosi): We should pass StatusBar object to
  // ContentWindow::UpdateStatusBar.
  public: virtual void UpdateStatusBar() const = 0;

  // [W]
  protected: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(ContentWindow);
};

}  // namespace content

#endif //!defined(INCLUDE_evita_content_content_window_h)
