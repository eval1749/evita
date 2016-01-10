// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_VISUAL_WINDOW_H_
#define EVITA_VIEWS_VISUAL_WINDOW_H_

#include "evita/views/canvas_content_window.h"

namespace visuals {
class DisplayItemList;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
class VisualWindow final : public CanvasContentWindow {
 public:
  explicit VisualWindow(WindowId window_id);
  ~VisualWindow() final;

  void Paint(std::unique_ptr<visuals::DisplayItemList> display_item_list);

 private:
  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // views::ContentWindow
  void MakeSelectionVisible() final;

  DISALLOW_COPY_AND_ASSIGN(VisualWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_VISUAL_WINDOW_H_
