// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_WINDOW_H_
#define EVITA_VIEWS_WINDOW_H_

#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/widget.h"
#include "evita/views/events/event_source.h"
#include "evita/views/window_id.h"

namespace views {

class Window : public ui::AnimatableWindow,
               protected EventSource {
  DECLARE_CASTABLE_CLASS(Window, ui::AnimatableWindow);

 public:
  int active_tick() const { return active_tick_; }
  WindowId window_id() const { return window_id_; }

  void DidDestroyDomWindow();
  static Window* FromWindowId(WindowId window_id);

 protected:
  typedef common::win::NativeWindow NativeWindow;

  Window(std::unique_ptr<NativeWindow> native_window, WindowId window_id);
  explicit Window(WindowId window_id = kInvalidWindowId);
  ~Window() override;

  // ui::Widget
  void DidChangeBounds() override;
  void DidHide() override;
  void DidKillFocus(ui::Widget* focused_window) override;
  void DidRealize() override;
  void DidSetFocus(ui::Widget* last_focused) override;
  void DidShow() override;
  void OnKeyPressed(const ui::KeyEvent& event) override;
  void OnKeyReleased(const ui::KeyEvent& event) override;
  void OnMouseMoved(const ui::MouseEvent& event) override;
  void OnMousePressed(const ui::MouseEvent& event) override;
  void OnMouseReleased(const ui::MouseEvent& event) override;
  void OnMouseWheel(const ui::MouseWheelEvent& event) override;
  void WillDestroyWidget() override;

 private:
  int active_tick_;
  const views::WindowId window_id_;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace views

#endif  // EVITA_VIEWS_WINDOW_H_
