// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_
#define EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_

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
// DocumentLifecycle
//
class DocumentLifecycle final {
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

  DocumentLifecycle();
  ~DocumentLifecycle();

  State state() const { return state_; }

  bool AllowsTreeMutaions() const;
  bool IsAtLeast(State state) const;
  void Reset();

 private:
  void AdvanceTo(State state);

  State state_;

  DISALLOW_COPY_AND_ASSIGN(DocumentLifecycle);
};

std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle& lifecycle);
std::ostream& operator<<(std::ostream& ostream, DocumentLifecycle::State state);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_LIFECYCLE_H_
