// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_scroll_bar_h)
#define INCLUDE_evita_ui_controls_scroll_bar_h

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
class ScrollBar : public ui::Widget {
  DECLARE_CASTABLE_CLASS(ScrollBar, Widget);

  public: struct Data {
    int minimum;
    int maximum;
    int thumb_value;
    int thumb_size;

    Data();

    public: bool operator==(const Data&) const;
    public: bool operator!=(const Data&) const;
  };

  public: class HitTestResult;
  public: enum class Location;

  // Note: class |Part| is base class of scroll bar parts, e.g. arrow, and
  // thumb. You can't use |Part| outside "scroll_bar.cc".
  public: class Part;

  public: enum class Type {
    Horizontal,
    Vertical,
  };

  private: Location capturing_location_;
  private: Part* capturing_part_;
  private: Data data_;
  private: Part* hover_part_;
  private: ScrollBarObserver* observer_;
  private: std::vector<Part*> parts_;

  public: ScrollBar(Type type, ScrollBarObserver* observer);
  public: virtual ~ScrollBar();

  private: static std::vector<Part*> CreateParts(Type type);
  private: HitTestResult HitTest(const gfx::PointF& point) const;
  private: void ResetHover();
  public: void SetData(const Data& date);
  private: void UpdateLayout();

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidShow() override;
  private: virtual void OnDraw(gfx::Canvas* canvas) override;
  private: virtual void OnMouseExited(const MouseEvent& event) override;
  private: virtual void OnMouseMoved(const MouseEvent& event) override;
  private: virtual void OnMousePressed(const MouseEvent& event) override;
  private: virtual void OnMouseReleased(const MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_scroll_bar_h)
