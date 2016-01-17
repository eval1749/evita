// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_
#define EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_

#include <iosfwd>

#include "base/macros.h"
#include "evita/visuals/dom/document_observer.h"

namespace visuals {

#define FOR_EACH_TREE_LIFECYCLE_STATE(V) \
  V(VisualUpdatePending)                 \
  V(InLayout)                            \
  V(LayoutClean)                         \
  V(InPaint)                             \
  V(PaintClean)

class Document;

//////////////////////////////////////////////////////////////////////
//
// DocumentLifecycle
//
class DocumentLifecycle final : public DocumentObserver {
 public:
  enum class State {
#define V(name) name,
    FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };

  class Scope {
   public:
    Scope(DocumentLifecycle* lifecycle, State from_state, State to_state);
    ~Scope();

   private:
    DocumentLifecycle* const lifecycle_;
    State const to_state_;

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  explicit DocumentLifecycle(const Document& document);
  ~DocumentLifecycle();

  const Document& document() const { return document_; }
  State state() const { return state_; }

  bool AllowsTreeMutaions() const;
  bool IsAtLeast(State state) const;
  void LimitTo(State state);
  void Reset();

 private:
  void AdvanceTo(State state);

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

  DISALLOW_COPY_AND_ASSIGN(DocumentLifecycle);
};

std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle& lifecycle);
std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle* lifecycle);
std::ostream& operator<<(std::ostream& ostream, DocumentLifecycle::State state);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_
