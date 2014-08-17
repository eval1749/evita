// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_animator_h)
#define INCLUDE_evita_views_tab_strip_animator_h

#include <memory>

#include "base/basictypes.h"

namespace ui {
class Animatables;
}

namespace views {

class TabContent;
class TabStrip;

class TabStripAnimator final {
  public: class Animation;

  private: const std::unique_ptr<ui::Animatables> animations_;
  private: TabStrip* const tab_strip_;

  public: TabStripAnimator(TabStrip* tab_strip);
  public: ~TabStripAnimator();

  public: void AddTab(TabContent* tab_content);
  private: void RegisterAnimation(Animation* animation);

  DISALLOW_COPY_AND_ASSIGN(TabStripAnimator);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_strip_animator_h)
