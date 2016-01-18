// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_MODEL_H_
#define EVITA_VISUALS_DEMO_DEMO_MODEL_H_

#include <memory>

#include "base/macros.h"
#include "evita/gc/member.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/media_state.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {

class DemoWindow;
class Document;
class ElementNode;
class View;

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
  ElementNode* FindListItem(const FloatPoint& point);

  // css::Media
  css::MediaState media_state() const final;
  css::MediaType media_type() const final;
  FloatSize viewport_size() const final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;
  const char* GetAnimationFrameType() const final;

  // WindowEventHandler
  void DidChangeWindowBounds(const FloatRect& bounds) final;
  void DidKillFocus() final;
  void DidMoveMouse(const FloatPoint& point) final;
  void DidPressMouse(const FloatPoint& point) final;
  void DidSetFocus() final;

  const gc::Member<Document> document_;
  const gc::Member<css::StyleSheet> style_sheet_;
  FloatSize viewport_size_;
  std::unique_ptr<View> view_;
  DemoWindow* window_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(DemoModel);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_MODEL_H_
