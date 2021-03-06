// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_VIEW_H_
#define EVITA_VISUALS_VIEW_VIEW_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/gfx/base/geometry/float_quad.h"
#include "evita/visuals/view/public/hit_test_result.h"
#include "evita/visuals/view/public/view_lifecycle_observer.h"

namespace css {
class Media;
class StyleSheet;
}

namespace visuals {

class BoxTree;
class DisplayItemList;
class Document;
class Node;
class Selection;
class StyleTree;
class UserActionSource;
class ViewLifecycle;
class ViewObserver;

//////////////////////////////////////////////////////////////////////
//
// View
//
class View final : public ViewLifecycleObserver {
 public:
  View(const Document& document,
       const css::Media& media,
       const UserActionSource& user_action_source,
       const std::vector<css::StyleSheet*>& style_sheets);
  ~View() final;

  const BoxTree& box_tree() const { return *box_tree_; }
  const Document& document() const;
  const ViewLifecycle& lifecycle() const { return *lifecycle_; }
  ViewLifecycle* lifecycle() { return lifecycle_.get(); }
  const css::Media& media() const;
  const Selection& selection() const { return *selection_; }
  Selection* selection() { return selection_.get(); }
  const StyleTree& style_tree() const { return *style_tree_; }

  void AddObserver(ViewObserver* observer) const;
  gfx::FloatQuad ComputeBorderBoxQuad(const Node& node);
  HitTestResult HitTest(const gfx::FloatPoint& point);
  std::unique_ptr<DisplayItemList> Paint();
  void ScheduleForcePaint();
  void RemoveObserver(ViewObserver* observer) const;

 private:
  // ViewLifecycleObserver
  void DidChangeLifecycleState(ViewLifecycle::State new_state,
                               ViewLifecycle::State old_state) final;

  void UpdateLayoutIfNeeded();
  void UpdateStyleIfNeeded();

  std::unique_ptr<ViewLifecycle> lifecycle_;
  mutable base::ObserverList<ViewObserver> observers_;
  std::unique_ptr<Selection> selection_;

  // |StyleTree| takes |ViewLifecycle| and |css::Media|.
  std::unique_ptr<StyleTree> style_tree_;

  // |BoxTree| takes |ViewLifecycle|, |Selection| and |StyleTree|.
  std::unique_ptr<BoxTree> box_tree_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_VIEW_H_
