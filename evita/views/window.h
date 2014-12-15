// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_window_h)
#define INCLUDE_evita_views_window_h

#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/widget.h"
#include "evita/views/event_source.h"
#include "evita/views/window_id.h"

namespace views {

class Window : public ui::AnimatableWindow,
               protected EventSource {
  DECLARE_CASTABLE_CLASS(Window, ui::AnimatableWindow);

  protected: typedef common::win::NativeWindow NativeWindow;

  private: int active_tick_;
  private: const views::WindowId window_id_;

  protected: Window(
      std::unique_ptr<NativeWindow> native_window, WindowId window_id);
  protected: explicit Window(WindowId window_id = kInvalidWindowId);
  protected: ~Window() override;

  public: int active_tick() const { return active_tick_; }
  public: WindowId window_id() const { return window_id_; }

  public: void DidDestroyDomWindow();
  public: static Window* FromWindowId(WindowId window_id);

  // ui::Widget
  protected: void DidChangeBounds() override;
  protected: void DidHide() override;
  protected: void DidKillFocus(ui::Widget* focused_window) override;
  protected: void DidRealize() override;
  protected: void DidSetFocus(ui::Widget* last_focused) override;
  protected: void DidShow() override;
  protected: void OnKeyPressed(const ui::KeyEvent& event) override;
  protected: void OnKeyReleased(const ui::KeyEvent& event) override;
  protected: void OnMouseMoved(const ui::MouseEvent& event) override;
  protected: void OnMousePressed(const ui::MouseEvent& event) override;
  protected: void OnMouseReleased(const ui::MouseEvent& event) override;
  protected: void OnMouseWheel(const ui::MouseWheelEvent& event) override;
  protected: void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_window_h)
