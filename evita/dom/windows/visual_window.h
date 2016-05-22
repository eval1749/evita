// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_
#define EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_

#include <memory>

#include "evita/dom/windows/window.h"

#include "base/observer_list.h"
#include "evita/css/media.h"
#include "evita/gfx/base/geometry/float_size.h"
#include "evita/visuals/view/public/user_action_source.h"
#include "evita/visuals/view/view_observer.h"

namespace css {
class StyleSheet;
}

namespace visuals {
class Document;
class View;
}

namespace dom {

class CSSStyleSheetHandle;
class ExceptionState;
class NodeHandle;

namespace bindings {
class VisualWindowClass;
}

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
class VisualWindow final : public ginx::Scriptable<VisualWindow, Window>,
                           public css::Media,
                           public visuals::UserActionSource,
                           public visuals::ViewObserver {
  DECLARE_SCRIPTABLE_OBJECT(VisualWindow);

 public:
  ~VisualWindow() final;

 private:
  friend class bindings::VisualWindowClass;

  VisualWindow(ScriptHost* script_host,
               visuals::Document* document,
               css::StyleSheet* style_sheet);

  const visuals::Document& document() const;

  void CancelAnimationFrame();
  void DidBeginAnimationFrame(const base::TimeTicks& now);
  void RequestAnimationFrame();

  // Binding callbacks
  int HitTest(int x, int y);
  static VisualWindow* NewWindow(ScriptHost* view_delegate,
                                 NodeHandle* document_handle,
                                 CSSStyleSheetHandle* style_sheet_handle,
                                 ExceptionState* exception_state);

  // ViewEventTarget
  bool HandleMouseEvent(const domapi::MouseEvent& event) final;

  // css::Media
  css::MediaState media_state() const final;
  css::MediaType media_type() const final;
  gfx::FloatSize viewport_size() const final;

  // visuals::UserActionSource
  void AddObserver(UserActionSource::Observer* observer) const final;
  void RemoveObserver(UserActionSource::Observer* observer) const final;

  // visuals::ViewObserver
  void DidChangeView() final;

  // Window
  void DidChangeBounds(int left, int top, int right, int bottom) final;
  void DidDestroyWindow() final;
  void DidShowWindow() final;

  int animation_request_id_ = 0;
  visuals::Node* hovered_node_ = nullptr;
  mutable base::ObserverList<UserActionSource::Observer> observers_;
  const std::unique_ptr<visuals::View> view_;
  gfx::FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(VisualWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_
