// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_MODEL_H_
#define EVITA_VISUALS_DEMO_DEMO_MODEL_H_

#include <memory>

#include "base/macros.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/visuals/demo/demo_window.h"

namespace visuals {

class Box;
class DemoWindow;
class PaintTracker;

//////////////////////////////////////////////////////////////////////
//
// DemoModel
//
class DemoModel final : public ui::AnimationFrameHandler,
                        public WindowEventHandler {
 public:
  DemoModel();
  ~DemoModel() final;

  void AttachWindow(DemoWindow* window);

 private:
  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;
  const char* GetAnimationFrameType() const final;

  // WindowEventHandler
  void DidChangeWindowBounds(const FloatRect& bounds) final;
  void DidMoveMouse(const FloatPoint& point) final;

  const std::unique_ptr<Box> root_box_;

  // |PaintTracker| takes |root_box_| as a constructor parameter.
  const std::unique_ptr<PaintTracker> paint_tracker_;
  DemoWindow* window_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(DemoModel);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_MODEL_H_
