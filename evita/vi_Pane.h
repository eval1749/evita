// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_vi_Pane_h)
#define INCLUDE_vi_Pane_h

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/ui/widget.h"

namespace views {
class Window;
}

class Frame;

class Pane : public ui::Widget {
  DECLARE_CASTABLE_CLASS(Pane, Widget);

  private: int active_tick_;

  protected: Pane();
  public: virtual ~Pane();

  public: int active_tick() const { return active_tick_; }

  // [A]
  public: virtual void Activate();

  // [G]
  protected: Frame* GetFrame() const;
  public: virtual views::Window* GetWindow() const { return nullptr; }

  // [U]
  public: void UpdateActiveTick();

  // ui::Widget
  protected: virtual void DidChangeBounds() override;
  protected: virtual void DidRealize() override;

  DISALLOW_COPY_AND_ASSIGN(Pane);
};

#endif //!defined(INCLUDE_vi_Pane_h)
