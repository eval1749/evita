// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_TREE_LIFECYCLE_H_
#define EVITA_VISUALS_MODEL_TREE_LIFECYCLE_H_

#include <iosfwd>

#include "base/macros.h"

namespace visuals {

#define FOR_EACH_TREE_LIFECYCLE_STATE(V) \
  V(VisualUpdatePending)                 \
  V(InLayout)                            \
  V(LayoutClean)                         \
  V(InPaint)                             \
  V(PaintClean)

//////////////////////////////////////////////////////////////////////
//
// TreeLifecycle
//
class TreeLifecycle final {
 public:
  enum class State {
#define V(name) name,
    FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };

  class Scope {
   public:
    Scope(TreeLifecycle* lifecycle, State from_state, State to_state);
    ~Scope();

   private:
    TreeLifecycle* const lifecycle_;
    State const to_state_;

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  TreeLifecycle();
  ~TreeLifecycle();

  State state() const { return state_; }

  bool AllowsTreeMutaions() const;
  bool IsAtLeast(State state) const;
  void Reset();

 private:
  void AdvanceTo(State state);

  State state_;

  DISALLOW_COPY_AND_ASSIGN(TreeLifecycle);
};

std::ostream& operator<<(std::ostream& ostream, const TreeLifecycle& lifecycle);
std::ostream& operator<<(std::ostream& ostream, TreeLifecycle::State state);

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_TREE_LIFECYCLE_H_
