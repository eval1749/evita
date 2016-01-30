// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tabs/tab_strip_animator.h"

#include "common/castable.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animatables.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_animation.h"
#include "evita/views/tabs/tab_content.h"
#include "evita/views/tabs/tab_content_observer.h"
#include "evita/views/tabs/tab_strip.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// TabStripAnimator::Action
//
class TabStripAnimator::Action : public common::Castable<Action>,
                                 private ui::AnimationObserver,
                                 private TabContentObserver {
  DECLARE_CASTABLE_CLASS(Action, Castable);

 public:
  virtual ~Action();

  void Cancel();

 protected:
  Action(TabStripAnimator* tab_strip_animator, TabContent* tab_content);

  TabContent* active_tab_content() const;
  base::TimeDelta animation_duration() const;
  ui::Layer* layer() const;
  TabStrip* tab_strip() const;

  virtual ui::Animatable* CreateAnimation() = 0;
  virtual void DoCancel() = 0;
  virtual void DoFinish() = 0;
  int FindTab(TabContent* tab_content) const;
  void SetActiveTabContent(TabContent* new_tab_content);

 private:
  // TabContentObserver
  void DidUpdateContent(TabContent* tab_content) final;

  // ui::AnimationObserver
  void DidCancelAnimation(ui::Animatable* animatable) final;
  void DidFinishAnimation(ui::Animatable* animatable) final;

  ui::Animatable* animation_;
  TabContent* observing_tab_content_;
  TabStripAnimator* const tab_strip_animator_;

  DISALLOW_COPY_AND_ASSIGN(Action);
};

TabStripAnimator::Action::Action(TabStripAnimator* tab_strip_animator,
                                 TabContent* tab_content)
    : animation_(nullptr),
      observing_tab_content_(tab_content),
      tab_strip_animator_(tab_strip_animator) {
  observing_tab_content_->AddObserver(this);
}

TabStripAnimator::Action::~Action() {
  if (observing_tab_content_) {
    DCHECK(!animation_);
    observing_tab_content_->RemoveObserver(this);
    return;
  }
  if (!animation_)
    return;
  animation_->RemoveObserver(this);
  animation_->CancelAnimation();
}

TabContent* TabStripAnimator::Action::active_tab_content() const {
  return tab_strip_animator_->active_tab_content();
}

base::TimeDelta TabStripAnimator::Action::animation_duration() const {
  return base::TimeDelta::FromMilliseconds(16 * 7);
}

ui::Layer* TabStripAnimator::Action::layer() const {
  return tab_strip_animator_->layer_;
}

TabStrip* TabStripAnimator::Action::tab_strip() const {
  return tab_strip_animator_->tab_strip_;
}

void TabStripAnimator::Action::Cancel() {
  if (observing_tab_content_) {
    DCHECK(!animation_);
    DCHECK(observing_tab_content_->visible());
    observing_tab_content_->RemoveObserver(this);
    observing_tab_content_->Hide();
    observing_tab_content_ = nullptr;
    return;
  }

  if (!animation_)
    return;
  animation_->CancelAnimation();
}

int TabStripAnimator::Action::FindTab(TabContent* tab_content) const {
  const auto num_tabs = tab_strip()->number_of_tabs();
  for (auto tab_index = 0; tab_index < num_tabs; ++tab_index) {
    if (tab_strip()->GetTab(tab_index) == tab_content)
      return tab_index;
  }
  return -1;
}

void TabStripAnimator::Action::SetActiveTabContent(
    TabContent* new_tab_content) {
  DCHECK_NE(active_tab_content(), new_tab_content);
  if (active_tab_content())
    active_tab_content()->Hide();
  tab_strip_animator_->active_tab_content_ = new_tab_content;
}

// TabContentObserver
void TabStripAnimator::Action::DidUpdateContent(TabContent* tab_content) {
  observing_tab_content_->RemoveObserver(this);
  observing_tab_content_ = nullptr;
  animation_ = CreateAnimation();
  if (!animation_) {
    tab_strip_animator_->DidFinishAction(this);
    return;
  }
  animation_->AddObserver(this);
  ui::Animator::instance()->ScheduleAnimation(animation_);
}

// ui::AnimationObserver
void TabStripAnimator::Action::DidCancelAnimation(ui::Animatable* animatable) {
  DCHECK_EQ(animation_, animatable);
  DoCancel();
  animation_ = nullptr;
  tab_strip_animator_->DidFinishAction(this);
}

void TabStripAnimator::Action::DidFinishAnimation(ui::Animatable* animatable) {
  DCHECK_EQ(animation_, animatable);
  DoFinish();
  animation_ = nullptr;
  tab_strip_animator_->DidFinishAction(this);
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// SelectTab
//
class SelectTabAction final : public TabStripAnimator::Action {
  DECLARE_CASTABLE_CLASS(SelectTabAction, Action);

 public:
  SelectTabAction(TabStripAnimator* tab_strip_animator,
                  TabContent* new_tab_content);
  ~SelectTabAction() final = default;

  TabContent* tab_content() const { return new_tab_content_; }

 private:
  // TabStripAnimator::Action
  ui::Animatable* CreateAnimation() final;
  void DoCancel() final;
  void DoFinish() final;

  TabContent* const new_tab_content_;
  TabContent* old_tab_content_;
  gfx::PointF old_tab_content_origin_;

  DISALLOW_COPY_AND_ASSIGN(SelectTabAction);
};

SelectTabAction::SelectTabAction(TabStripAnimator* tab_strip_animator,
                                 TabContent* new_tab_content)
    : Action(tab_strip_animator, new_tab_content),
      new_tab_content_(new_tab_content),
      old_tab_content_(nullptr) {}

// TabStripAnimator::Action
ui::Animatable* SelectTabAction::CreateAnimation() {
  old_tab_content_ = active_tab_content();
  DCHECK_NE(old_tab_content_, new_tab_content_);
  const auto old_layer = old_tab_content_ ? old_tab_content_->layer() : nullptr;
  const auto new_layer = new_tab_content_->layer();
  DCHECK(!new_layer->parent_layer());
  return ui::LayerAnimation::CreateSlideIn(layer(), new_layer, old_layer);
}

void SelectTabAction::DoCancel() {
  const auto new_layer = new_tab_content_->layer();
  if (new_layer->parent_layer()) {
    DCHECK_EQ(new_layer->parent_layer(), layer());
    layer()->RemoveLayer(new_layer);
  }
  if (!old_tab_content_ || old_tab_content_ != active_tab_content())
    return;
  old_tab_content_->layer()->SetOrigin(old_tab_content_origin_);
}

void SelectTabAction::DoFinish() {
  const auto tab_index = FindTab(new_tab_content_);
  if (tab_index < 0) {
    new_tab_content_->Hide();
    return;
  }
  SetActiveTabContent(new_tab_content_);
  tab_strip()->SelectTab(tab_index);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TabStripAnimator
//
TabStripAnimator::TabStripAnimator(TabStrip* tab_strip)
    : action_(nullptr),
      active_tab_content_(nullptr),
      layer_(nullptr),
      tab_strip_(tab_strip) {}

TabStripAnimator::~TabStripAnimator() {
  delete action_;
}

void TabStripAnimator::AddTab(TabContent* tab_content) {
  DCHECK(layer_);
  // Set dummy tab label. Actual tab label will be set later in
  // |Frame::UpdateTitleBar|.
  const auto new_tab_item_index = tab_strip_->number_of_tabs();
  tab_strip_->InsertTab(tab_content, new_tab_item_index);
  RequestSelect(tab_content);
}

void TabStripAnimator::CancelCurrentAction() {
  const auto current_action = action_;
  if (!current_action)
    return;
  action_ = nullptr;
  current_action->Cancel();
}

void TabStripAnimator::DidDeleteTabContent(TabContent* tab_content) {
  if (active_tab_content_ != tab_content)
    return;
  active_tab_content_ = nullptr;
}

void TabStripAnimator::DidFinishAction(Action* action) {
  delete action;
  if (action_ == action)
    action_ = nullptr;
}

void TabStripAnimator::RequestSelect(TabContent* tab_content) {
  if (action_) {
    if (auto select_action = action_->as<SelectTabAction>()) {
      if (select_action->tab_content() == tab_content)
        return;
      CancelCurrentAction();
    }
  }
  if (active_tab_content_ == tab_content)
    return;
  action_ = new SelectTabAction(this, tab_content);
  if (tab_content->visible()) {
    // During realization, |tab_content| is already visible.
    return;
  }
  tab_content->Show();
}

void TabStripAnimator::SetTabContentLayer(ui::Layer* layer) {
  DCHECK(layer);
  DCHECK(!layer_);
  layer_ = layer;
}

}  // namespace views
