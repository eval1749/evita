// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_MODEL_H_
#define EVITA_VISUALS_DEMO_DEMO_MODEL_H_

#include <memory>

#include "base/macros.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/model/box_finder.h"

namespace visuals {

class BoxTree;
class DemoWindow;
class Document;
class Element;
class StyleTree;

namespace css {
class StyleSheet;
}

//////////////////////////////////////////////////////////////////////
//
// DemoModel
//
class DemoModel final : public css::Media,
                        public ui::AnimationFrameHandler,
                        public WindowEventHandler {
 public:
  DemoModel();
  ~DemoModel() final;

  void AttachWindow(DemoWindow* window);

 private:
  Element* FindListItem(const FloatPoint& point);
  void UpdateLayoutIfNeeded();
  void UpdateStyleIfNeeded();

  // css::Media
  css::MediaType media_type() const final;
  FloatSize viewport_size() const final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;
  const char* GetAnimationFrameType() const final;

  // WindowEventHandler
  void DidChangeWindowBounds(const FloatRect& bounds) final;
  void DidMoveMouse(const FloatPoint& point) final;
  void DidPressMouse(const FloatPoint& point) final;

  Document* const document_;
  css::StyleSheet* style_sheet_;
  FloatSize viewport_size_;

  std::unique_ptr<StyleTree> style_tree_;
  std::unique_ptr<BoxTree> box_tree_;

  DemoWindow* window_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(DemoModel);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_MODEL_H_
