// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_SCROLL_BAR_H_
#define EVITA_UI_CONTROLS_SCROLL_BAR_H_

#include <vector>

#include "evita/ui/widget.h"

namespace gfx {
class Canvas;
}

namespace ui {

class ScrollBarObserver;

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
class ScrollBar final : public ui::Widget {
  DECLARE_CASTABLE_CLASS(ScrollBar, Widget);

 public:
  struct Data final {
    int minimum;
    int maximum;
    int thumb_value;
    int thumb_size;

    Data();

    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
  };

  class HitTestResult;
  enum class Location;

  // class |Part| is base class of scroll bar parts, e.g. arrow, and
  // thumb. You can't use |Part| outside "scroll_bar.cc".
  class Part;

  enum class Type {
    Horizontal,
    Vertical,
  };

  ScrollBar(Type type, ScrollBarObserver* observer);
  ~ScrollBar();

  void SetData(const Data& date);

 private:
  static std::vector<Part*> CreateParts(Type type);
  HitTestResult HitTest(const gfx::PointF& point) const;
  void ResetHover();
  void UpdateLayout();

  // ui::Widget
  void DidChangeBounds() final;
  void DidShow() final;
  void OnDraw(gfx::Canvas* canvas) final;
  void OnMouseExited(const MouseEvent& event) final;
  void OnMouseMoved(const MouseEvent& event) final;
  void OnMousePressed(const MouseEvent& event) final;
  void OnMouseReleased(const MouseEvent& event) final;

  Location capturing_location_;
  Part* capturing_part_;
  Data data_;
  Part* hover_part_;
  ScrollBarObserver* observer_;
  std::vector<Part*> parts_;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_SCROLL_BAR_H_
