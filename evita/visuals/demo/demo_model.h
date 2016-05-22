// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_MODEL_H_
#define EVITA_VISUALS_DEMO_DEMO_MODEL_H_

#include <memory>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/css/media.h"
#include "evita/css/media_state.h"
#include "evita/gc/member.h"
#include "evita/gfx/base/geometry/float_size.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/view/public/user_action_source.h"
#include "evita/visuals/view/view_observer.h"

namespace css {
class StyleSheet;
}

namespace visuals {

class DemoWindow;
class Document;
class ElementNode;
class Selection;
class View;

//////////////////////////////////////////////////////////////////////
//
// DemoModel
//
class DemoModel final : public css::Media,
                        public ui::AnimationFrameHandler,
                        public visuals::UserActionSource,
                        public ViewObserver,
                        public WindowEventHandler {
 public:
  DemoModel();
  ~DemoModel() final;

  void AttachWindow(DemoWindow* window);

 private:
  Selection* selection();

  ElementNode* FindListItem(const gfx::FloatPoint& point);

  // css::Media
  css::MediaState media_state() const final;
  css::MediaType media_type() const final;
  gfx::FloatSize viewport_size() const final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;
  const char* GetAnimationFrameType() const final;

  // visuals::UserActionSource
  void AddObserver(UserActionSource::Observer* observer) const final;
  void RemoveObserver(UserActionSource::Observer* observer) const final;

  // ViewObserver
  void DidChangeView() final;

  // WindowEventHandler
  void DidChangeWindowBounds(const gfx::FloatRect& bounds) final;
  void DidKillFocus() final;
  void DidMoveMouse(const gfx::FloatPoint& point) final;
  void DidPressKey(int key_code) final;
  void DidPressMouse(const gfx::FloatPoint& point) final;
  void DidSetFocus() final;

  const gc::Member<Document> document_;
  Node* hovered_node_ = nullptr;
  mutable base::ObserverList<UserActionSource::Observer> observers_;
  const gc::Member<css::StyleSheet> style_sheet_;
  gfx::FloatSize viewport_size_;
  std::unique_ptr<View> view_;
  DemoWindow* window_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(DemoModel);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_MODEL_H_
