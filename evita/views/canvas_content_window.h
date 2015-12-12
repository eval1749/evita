// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_CANVAS_CONTENT_WINDOW_H_
#define EVITA_VIEWS_CANVAS_CONTENT_WINDOW_H_

#include <memory>

#include "evita/views/content_window.h"

class Frame;

namespace gfx {
class Canvas;
}

namespace views {

class CanvasContentWindow : public ContentWindow {
  DECLARE_CASTABLE_CLASS(CanvasContentWindow, ContentWindow);

 public:
  ~CanvasContentWindow() override;

  virtual void MakeSelectionVisible() = 0;
  void RemoveObserver(ContentObserver* observer);

 protected:
  explicit CanvasContentWindow(views::WindowId window_id);

  gfx::Canvas* canvas() { return canvas_.get(); }
  const gfx::Canvas* canvas() const { return canvas_.get(); }

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) override;

  // ui::Widget
  void DidChangeBounds() override;
  void DidHide() override;
  void DidShow() override;
  void OnDraw(gfx::Canvas* canvas) override;

 private:
  std::unique_ptr<gfx::Canvas> canvas_;

  DISALLOW_COPY_AND_ASSIGN(CanvasContentWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_CANVAS_CONTENT_WINDOW_H_
