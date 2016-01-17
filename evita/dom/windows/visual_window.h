// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_
#define EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_

#include <memory>

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/geometry/float_size.h"

namespace base {
class Time;
}

namespace visuals {
namespace css {
class Style;
class StyleSheet;
}
class BoxTree;
class ContainerNode;
class Document;
class DocumentLifecycle;
class ElementNode;
class Node;
class Selection;
class StyleTree;
class Text;
}  // namespace visuals

namespace dom {

class NodeHandle;
class CSSStyleSheetHandle;

namespace bindings {
class VisualWindowClass;
}

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
class VisualWindow final : public v8_glue::Scriptable<VisualWindow, Window>,
                           public visuals::css::Media,
                           public visuals::DocumentObserver {
  DECLARE_SCRIPTABLE_OBJECT(VisualWindow);

 public:
  ~VisualWindow() final;

 private:
  friend class bindings::VisualWindowClass;
  using ContainerNode = visuals::ContainerNode;
  using CssMedia = visuals::css::Media;
  using CssStyle = visuals::css::Style;
  using CssStyleSheet = visuals::css::StyleSheet;
  using ElementNode = visuals::ElementNode;
  using FloatSize = visuals::FloatSize;
  using Node = visuals::Node;
  using Text = visuals::Text;

  VisualWindow(visuals::Document* document,
               visuals::css::StyleSheet* style_sheet);

  void DidBeginAnimationFrame(const base::TimeTicks& now);
  void RequestAnimationFrame();
  void UpdateLayoutIfNeeded();
  void UpdateStyleIfNeeded();

  // Binding callbacks
  int HitTest(int x, int y);
  static VisualWindow* NewWindow(NodeHandle* document_handle,
                                 CSSStyleSheetHandle* style_sheet_handle);

  // visuals::css::Media
  visuals::css::MediaState media_state() const final;
  visuals::css::MediaType media_type() const final;
  visuals::FloatSize viewport_size() const final;

  // visuals::DocumentObserver
  void DidAddClass(const ElementNode& element,
                   const base::string16& new_name) final;
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const ElementNode& element,
                            const CssStyle* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidRemoveChild(const ContainerNode& parent, const Node& child) final;
  void DidRemoveClass(const ElementNode& element,
                      const base::string16& old_name) final;
  void DidReplaceChild(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidSetTextData(const Text& text,
                      const base::string16& new_data,
                      const base::string16& old_data) final;
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  // Window
  void DidChangeBounds(int left, int top, int right, int bottom) final;
  void DidRealizeWindow() final;
  void DidShowWindow() final;

  bool is_waiting_animation_frame_ = false;
  const std::unique_ptr<visuals::DocumentLifecycle> lifecycle_;
  const std::unique_ptr<visuals::Selection> selection_;
  const std::unique_ptr<visuals::StyleTree> style_tree_;
  FloatSize viewport_size_;

  // |BoxTree| takes |StyleTree|
  const std::unique_ptr<visuals::BoxTree> box_tree_;

  DISALLOW_COPY_AND_ASSIGN(VisualWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_VISUAL_WINDOW_H_
