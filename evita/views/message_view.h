// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_message_view_h)
#define INCLUDE_evita_views_message_view_h

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/ui/animation/animatable_window.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// MessageView
//
class MessageView final : public ui::AnimatableWindow {
  DECLARE_CASTABLE_CLASS(MessageView, ui::AnimatableWindow);

  private: class View;

  private: std::unique_ptr<View> view_;

  public: MessageView();
  public: virtual ~MessageView();

  public: void SetMessage(const base::string16& text);
  public: void SetStatus(const std::vector<base::string16>& texts);

  // ui::AnimationFrameHandler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::Widget
  private: virtual void DidRealize() override;
  private: virtual gfx::Size GetPreferredSize() const override;

  DISALLOW_COPY_AND_ASSIGN(MessageView);
};

}  // namespace views

#endif // !defined(INCLUDE_evita_views_message_view_h)
