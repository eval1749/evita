// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_strip_animator.h"

#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animatables.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_animation.h"
#include "evita/views/tab_content.h"
#include "evita/views/tab_content_observer.h"
#include "evita/views/tab_strip.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// TabStripAnimator::Action
//
class TabStripAnimator::Action : private ui::AnimationObserver,
                                 private TabContentObserver {
  private: ui::Animatable* animation_;
  private: TabContent* tab_content_;
  private: TabStripAnimator* const tab_strip_animator_;

  protected: Action(TabStripAnimator* tab_strip_animator,
                    TabContent* tab_content);
  public: virtual ~Action();

  protected: TabContent* active_tab_content() const;
  protected: base::TimeDelta animation_duration() const;
  protected: ui::Layer* layer() const;
  protected: TabStrip* tab_strip() const;

  public: void Cancel();
  protected: virtual ui::Animatable* CreateAnimation() = 0;
  protected: virtual void DoCancel() = 0;
  protected: virtual void DoFinish() = 0;
  protected: int FindTab(TabContent* tab_content) const;
  protected: void SetActiveTabContent(TabContent* new_tab_content);

  // TabContentObserver
  private: virtual void DidAnimateTabContent(TabContent* tab_content) override;

  // ui::AnimationObserver
  private: virtual void DidCancelAnimation(
      ui::Animatable* animatable) override;
  private: virtual void DidFinishAnimation(
      ui::Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(Action);
};

TabStripAnimator::Action::Action(TabStripAnimator* tab_strip_animator,
                                 TabContent* tab_content)
    : animation_(nullptr), tab_content_(tab_content),
      tab_strip_animator_(tab_strip_animator) {
  tab_content_->AddObserver(this);
}

TabStripAnimator::Action::~Action() {
  if (tab_content_) {
    DCHECK(!animation_);
    tab_content_->RemoveObserver(this);
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
  if (tab_content_) {
    DCHECK(!animation_);
    tab_content_->RemoveObserver(this);
    tab_content_ = nullptr;
    return;
  }

  if (!animation_)
   return;
  animation_->CancelAnimation();
}

int TabStripAnimator::Action::FindTab(TabContent* tab_content) const {
  auto const num_tabs = tab_strip()->number_of_tabs();
  for (auto tab_index = 0; tab_index < num_tabs; ++tab_index) {
    TCITEM tab_item;
    tab_item.mask = TCIF_PARAM;
    if (!tab_strip()->GetTab(tab_index, &tab_item))
      continue;
    if (reinterpret_cast<TabContent*>(tab_item.lParam) == tab_content)
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
void TabStripAnimator::Action::DidAnimateTabContent(TabContent*) {
  tab_content_->RemoveObserver(this);
  tab_content_ = nullptr;
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
class SelectTabAction : public TabStripAnimator::Action {
  private: TabContent* const new_tab_content_;
  private: TabContent* old_tab_content_;
  private: gfx::PointF old_tab_content_origin_;

  public: SelectTabAction(TabStripAnimator* tab_strip_animator,
                          TabContent* new_tab_content);
  protected: virtual ~SelectTabAction() = default;

  // TabStripAnimator::Action
  private: virtual ui::Animatable* CreateAnimation() override;
  private: virtual void DoCancel() override;
  private: virtual void DoFinish() override;

  DISALLOW_COPY_AND_ASSIGN(SelectTabAction);
};

SelectTabAction::SelectTabAction(TabStripAnimator* tab_strip_animator,
                                 TabContent* new_tab_content)
    : Action(tab_strip_animator, new_tab_content),
      new_tab_content_(new_tab_content), old_tab_content_(nullptr) {
}

// TabStripAnimator::Action
ui::Animatable* SelectTabAction::CreateAnimation() {
  old_tab_content_ = active_tab_content();
  DCHECK_NE(old_tab_content_, new_tab_content_);
  auto const new_layer = new_tab_content_->layer();
  new_layer->SetOrigin(gfx::PointF(-new_layer->bounds().width(),
                                   new_layer->bounds().top));
  layer()->AppendLayer(new_layer);
  if (!old_tab_content_) {
    return ui::LayerAnimation::CreateSlideInFromLeft(
        new_layer, nullptr);
  }
  old_tab_content_origin_ = old_tab_content_->layer()->bounds().origin();
  return ui::LayerAnimation::CreateSlideInFromLeft(
        new_layer, old_tab_content_->layer());
}

void SelectTabAction::DoCancel() {
  layer()->RemoveLayer(new_tab_content_->layer());
  if (!old_tab_content_ || old_tab_content_ != active_tab_content())
    return;
  old_tab_content_->layer()->SetOrigin(old_tab_content_origin_);
}

void SelectTabAction::DoFinish() {
  auto const tab_index = FindTab(new_tab_content_);
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
    : action_(nullptr), active_tab_content_(nullptr), layer_(nullptr),
      tab_strip_(tab_strip) {
}

TabStripAnimator::~TabStripAnimator() {
  delete action_;
}

void TabStripAnimator::AddTab(TabContent* tab_content) {
  DCHECK(layer_);
  // Set dummy tab label. Actual tab label will be set later in
  // |Frame::UpdateTitleBar|.
  TCITEM tab_item;
  tab_item.mask = TCIF_IMAGE| TCIF_TEXT | TCIF_PARAM;
  tab_item.pszText = L"?";
  tab_item.lParam = reinterpret_cast<LPARAM>(tab_content);
  tab_item.iImage = 0;
  auto const new_tab_item_index = tab_strip_->number_of_tabs();
  tab_strip_->InsertTab(new_tab_item_index, &tab_item);
  RegisterAction(new SelectTabAction(this, tab_content));
}

void TabStripAnimator::CancelCurrentAction() {
  auto const current_action = action_;
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

void TabStripAnimator::RegisterAction(Action* action) {
  CancelCurrentAction();
  action_ = action;
}

void TabStripAnimator::RequestSelect(TabContent* new_tab_content) {
  if (new_tab_content == active_tab_content_) {
    CancelCurrentAction();
    return;
  }
  RegisterAction(new SelectTabAction(this, new_tab_content));
  new_tab_content->Show();
}

void TabStripAnimator::SetLayer(ui::Layer* layer) {
  DCHECK(layer);
  DCHECK(!layer_);
  layer_ = layer;
}

}  // namespace views
