// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_
#define EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_

#include <iosfwd>

#include "base/macros.h"

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

namespace css {
class Media;
}

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycle
//
class ViewLifecycle final {
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

  ViewLifecycle(const Document& document, const css::Media& media);
  ~ViewLifecycle();

  bool operator==(const ViewLifecycle& other) const;
  bool operator==(const ViewLifecycle* other) const;
  bool operator!=(const ViewLifecycle& other) const;
  bool operator!=(const ViewLifecycle* other) const;

  const Document& document() const { return document_; }
  const css::Media& media() const { return media_; }
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
  void StartOver();
  void StartShutdown();

 private:
  void Advance();

  const Document& document_;
  const css::Media& media_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(ViewLifecycle);
};

std::ostream& operator<<(std::ostream& ostream, const ViewLifecycle& lifecycle);
std::ostream& operator<<(std::ostream& ostream, const ViewLifecycle* lifecycle);
std::ostream& operator<<(std::ostream& ostream, ViewLifecycle::State state);

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_VIEW_LIFECYCLE_H_
