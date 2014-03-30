// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_window_h)
#define INCLUDE_evita_views_window_h

#include "evita/ui/widget.h"
#include "evita/views/event_source.h"
#include "evita/views/window_id.h"

namespace views {

class Window : public ui::Widget, protected EventSource {
  DECLARE_CASTABLE_CLASS(Window, Widget);

  protected: typedef common::win::NativeWindow NativeWindow;

  private: int active_tick_;
  private: const views::WindowId window_id_;

  protected: Window(
      std::unique_ptr<NativeWindow>&& native_window, WindowId window_id);
  protected: explicit Window(WindowId window_id = kInvalidWindowId);
  protected: virtual ~Window();

  public: int active_tick() const { return active_tick_; }
  public: WindowId window_id() const { return window_id_; }

  public: void DidDestroyDomWindow();
  protected: virtual void DidKillFocus() override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidResize() override;
  protected: virtual void DidSetFocus() override;
  public: static Window* FromWindowId(WindowId window_id);
  public: virtual bool OnIdle(int hint);

  // ui::Widget
  protected: virtual void OnKeyPressed(
      const ui::KeyboardEvent& event) override;
  protected: virtual void OnKeyReleased(
      const ui::KeyboardEvent& event) override;
  protected: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  protected: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  protected: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  protected: virtual void OnMouseWheel(
      const ui::MouseWheelEvent& event) override;
  protected: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_window_h)
