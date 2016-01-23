// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_MESSAGE_VIEW_H_
#define EVITA_VIEWS_MESSAGE_VIEW_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/ui/animation/animatable_window.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// MessageView
//
class MessageView final : public ui::AnimatableWindow {
  DECLARE_CASTABLE_CLASS(MessageView, ui::AnimatableWindow);

 public:
  MessageView();
  ~MessageView() final;

  void SetMessage(base::StringPiece16 text);
  void SetStatus(const std::vector<base::string16>& texts);

 private:
  class View;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;

  // ui::Widget
  void DidRealize() final;
  gfx::Size GetPreferredSize() const final;
  void WillDestroyWidget() final;

  std::unique_ptr<View> view_;

  DISALLOW_COPY_AND_ASSIGN(MessageView);
};

}  // namespace views

#endif  // EVITA_VIEWS_MESSAGE_VIEW_H_
