// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_strip_animator.h"

#include "evita/ui/animation/animatable.h"
#include "evita/ui/animation/animatables.h"
#include "evita/ui/animation/animator.h"
#include "evita/views/tab_content.h"
#include "evita/views/tab_content_observer.h"
#include "evita/views/tab_strip.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// TabStripAnimator::Animation
//
class TabStripAnimator::Animation : public ui::Animatable,
                                    private TabContentObserver {
  private: TabContent* tab_content_;
  private: TabStrip* const tab_strip_;

  protected: Animation(TabStrip* tab_strip, TabContent* tab_content);
  public: virtual ~Animation();

  protected: TabStrip* tab_strip() const { return tab_strip_; }

  protected: int FindTab(TabContent* tab_content) const;

  // TabContentObserver
  private: virtual void DidAnimateTabContent(TabContent* tab_content) override;

  DISALLOW_COPY_AND_ASSIGN(Animation);
};

TabStripAnimator::Animation::Animation(TabStrip* tab_strip,
                                       TabContent* tab_content)
    : tab_content_(tab_content), tab_strip_(tab_strip) {
  tab_content_->AddObserver(this);
}

TabStripAnimator::Animation::~Animation() {
  if (tab_content_)
    tab_content_->RemoveObserver(this);
}

int TabStripAnimator::Animation::FindTab(TabContent* tab_content) const {
  auto const num_tabs = tab_strip_->number_of_tabs();
  for (auto tab_index = 0; tab_index < num_tabs; ++tab_index) {
    TCITEM tab_item;
    tab_item.mask = TCIF_PARAM;
    if (!tab_strip_->GetTab(tab_index, &tab_item))
      continue;
    if (reinterpret_cast<TabContent*>(tab_item.lParam) == tab_content)
      return tab_index;
  }
  return -1;
}

// TabContentObserver
void TabStripAnimator::Animation::DidAnimateTabContent(TabContent*) {
  tab_content_->RemoveObserver(this);
  tab_content_ = nullptr;
  ui::Animator::instance()->ScheduleAnimation(this);
}

namespace {

class SlideInAnimation final : public TabStripAnimator::Animation {
  private: TabContent* tab_content_;

  public: SlideInAnimation(TabStrip* tab_strip, TabContent* tab_content);
  private: virtual ~SlideInAnimation() = default;

  private: virtual void Animate(base::Time time) override;

  DISALLOW_COPY_AND_ASSIGN(SlideInAnimation);
};

SlideInAnimation::SlideInAnimation(TabStrip* tab_strip,
                                   TabContent* tab_content)
    : Animation(tab_strip, tab_content), tab_content_(tab_content) {
}

void SlideInAnimation::Animate(base::Time) {
  auto const tab_index = FindTab(tab_content_);
  if (tab_index < 0)
    return;
  tab_strip()->SelectTab(tab_index);
  FinishAnimation();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TabStripAnimator
//
TabStripAnimator::TabStripAnimator(TabStrip* tab_strip)
    : animations_(new ui::Animatables()), tab_strip_(tab_strip) {
}

TabStripAnimator::~TabStripAnimator() {
}

void TabStripAnimator::AddTab(TabContent* tab_content) {
  // Set dummy tab label. Actual tab label will be set later in
  // |Frame::UpdateTitleBar|.
  TCITEM tab_item;
  tab_item.mask = TCIF_IMAGE| TCIF_TEXT | TCIF_PARAM;
  tab_item.pszText = L"?";
  tab_item.lParam = reinterpret_cast<LPARAM>(tab_content);
  tab_item.iImage = 0;
  auto const new_tab_item_index = tab_strip_->number_of_tabs();
  tab_strip_->InsertTab(new_tab_item_index, &tab_item);
  RegisterAnimation(new SlideInAnimation(tab_strip_, tab_content));
}

void TabStripAnimator::RegisterAnimation(Animation* animation) {
  animations_->AddAnimatable(animation);
}

}  // namespace views
