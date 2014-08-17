// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_content_h)
#define INCLUDE_evita_views_tab_content_h

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/ui/widget.h"

class Frame;

namespace domapi {
struct TabData;
}

namespace views {
class Window;

class TabContent : public ui::Widget {
  DECLARE_CASTABLE_CLASS(TabContent, Widget);

  private: int active_tick_;

  protected: TabContent();
  public: virtual ~TabContent();

  public: int active_tick() const { return active_tick_; }

  public: virtual void Activate();
  protected: Frame* GetFrame() const;
  public: virtual const domapi::TabData* GetTabData() const = 0;

  // ui::Widget
  protected: virtual void DidRealize() override;

  DISALLOW_COPY_AND_ASSIGN(TabContent);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_content_h)
