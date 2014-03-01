// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_scroll_bar_h)
#define INCLUDE_evita_ui_controls_scroll_bar_h

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
  };

  public: enum class Type {
    Horizonal,
    Vertical,
  };

  private: HWND hwnd_;
  private: ScrollBarObserver* const observer_;
  private: Type const type_;

  public: ScrollBar(Type type, ScrollBarObserver* observer);
  public: ~ScrollBar();

  public: void SetData(const Data& date);

  // ui::Widget
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidHide() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidShow() override;
  private: virtual void OnScroll(int request) override;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_scroll_bar_h)
