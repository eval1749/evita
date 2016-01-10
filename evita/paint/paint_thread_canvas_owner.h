// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PAINT_THREAD_CANVAS_OWNER_H_
#define EVITA_PAINT_PAINT_THREAD_CANVAS_OWNER_H_

#include <memory>

#include "base/time/time.h"

namespace base {
class Time;
}

namespace ui {
class AnimatableWindow;
class Layer;
}

namespace gfx {
class Canvas;
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// PaintThreadCanvasOwner
//
class PaintThreadCanvasOwner {
 public:
  virtual ~PaintThreadCanvasOwner();

  void DidBeginAnimationFrame(const base::TimeTicks& now);
  void DidRealize();
  void DidRecreateParentLayer();
  void WillDestroyWidget();

 protected:
  explicit PaintThreadCanvasOwner(ui::AnimatableWindow* widget);

  virtual void PaintAnimationFrame(gfx::Canvas* canvas,
                                   const base::TimeTicks& now) = 0;
  void RequestAnimationFrame();

 private:
  class Impl;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(PaintThreadCanvasOwner);
};

}  // namespace paint

#endif  // EVITA_PAINT_PAINT_THREAD_CANVAS_OWNER_H_
