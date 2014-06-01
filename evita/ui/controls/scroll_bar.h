// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_scroll_bar_h)
#define INCLUDE_evita_ui_controls_scroll_bar_h

#include <vector>

#include "evita/gfx_base.h"
#include "evita/ui/widget.h"

namespace ui {

class ScrollBarObserver;

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

  // Note: class |Part| is base class of scroll bar parts, e.g. arrow, and
  // thumb. You can't use |Part| outside "scroll_bar.cc".
  public: class Part;

  public: enum class Type {
    Horizontal,
    Vertical,
  };

  private: Data data_;
  private: bool dirty_;
  private: Part* hover_part_;
  private: std::vector<Part*> parts_;
  private: Part* const thumb_;

  public: ScrollBar(Type type, ScrollBarObserver* observer);
  public: virtual ~ScrollBar();

  private: static std::vector<Part*> CreateParts(Type type,
                                                 ScrollBarObserver* observer);
  private: Part* HitTest(const gfx::Point& point) const;
  public: void SetData(const Data& date);
  private: void UpdateAppearance();
  private: void UpdateLayout();

  // ui::Widget
  private: virtual void DidResize() override;
  private: virtual void OnDraw(gfx::Graphics* canvas) override;
  private: virtual void OnMouseExited(const MouseEvent& event) override;
  private: virtual void OnMouseMoved(const MouseEvent& event) override;
  private: virtual void OnMousePressed(const MouseEvent& event) override;
  private: virtual void OnMouseReleased(const MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_scroll_bar_h)
