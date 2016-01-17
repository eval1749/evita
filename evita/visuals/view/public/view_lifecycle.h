// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_
#define EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_

#include <iosfwd>

#include "base/macros.h"
#include "evita/visuals/dom/document_observer.h"

namespace visuals {

#define FOR_EACH_TREE_LIFECYCLE_STATE(V) \
  V(VisualUpdatePending)                 \
  V(InStyleRecalc)                       \
  V(StyleClean)                          \
  V(InTreeRebuild)                       \
  V(TreeClean)                           \
  V(InLayout)                            \
  V(LayoutClean)                         \
  V(InPaint)                             \
  V(PaintClean)                          \
  V(InShutdown)                          \
  V(Shutdown)

class Document;

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycle
//
class ViewLifecycle final : public DocumentObserver {
 public:
  enum class State {
#define V(name) name,
    FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };

  class Scope {
   public:
    Scope(ViewLifecycle* lifecycle, State from_state);
    ~Scope();

   private:
    State const from_state_;
    ViewLifecycle* const lifecycle_;

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  explicit ViewLifecycle(const Document& document);
  ~ViewLifecycle();

  const Document& document() const { return document_; }
  State state() const { return state_; }

  bool AllowsSelectionChanges() const;
  bool AllowsTreeMutaions() const;
  void FinishShutdown();
  bool InShutdown() const { return state_ == State::InShutdown; }
  bool IsAtLeast(State state) const;
  bool IsStyleClean() const;
  bool IsTreeClean() const;
  bool IsLayoutClean() const;
  void LimitTo(State state);
  void Reset();
  void StartShutdown();

 private:
  void Advance();

  // DocumentObserver
  void DidAddClass(const ElementNode& element,
                   const base::string16& new_name) final;
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const ElementNode& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void DidRemoveChild(const ContainerNode& parent, const Node& child) final;
  void DidRemoveClass(const ElementNode& element,
                      const base::string16& old_name) final;
  void DidReplaceChild(const ContainerNode& parent,
                       const Node& new_child,
                       const Node& old_child) final;
  void DidSetTextData(const Text& text,
                      const base::string16& new_data,
                      const base::string16& old_data) final;
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  const Document& document_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(ViewLifecycle);
};

std::ostream& operator<<(std::ostream& ostream, const ViewLifecycle& lifecycle);
std::ostream& operator<<(std::ostream& ostream, const ViewLifecycle* lifecycle);
std::ostream& operator<<(std::ostream& ostream, ViewLifecycle::State state);

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_
