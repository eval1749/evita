// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_h)
#define INCLUDE_evita_views_tab_strip_h

#include <memory>

#include "evita/ui/widget.h"

const char16 TabBand__TabDragMsgStr[] = L"Evita.TabBand.TabDrag";

namespace ui {
class MouseEvent;
}

namespace views {

using common::win::Size;
using common::win::Rect;

class TabStripDelegate;

class TabStrip : public ui::Widget {
  DECLARE_CASTABLE_CLASS(TabStrip, Widget);

  private: class TabStripImpl;
  friend class TabStripImpl;

  private: TabStripDelegate* delegate_;
  private: std::unique_ptr<TabStripImpl> impl_;

  public: TabStrip(TabStripDelegate* delegate);
  public: virtual ~TabStrip();

  public: int number_of_tabs() const;
  public: int selected_index() const;

  public: Size GetPreferreSize() const;
  public: void DeleteTab(int tab_index);
  public: void InsertTab(int new_tab_index, const TCITEM* tab_data);
  public: void SelectTab(int tab_index);
  public: void SetIconList(HIMAGELIST icon_list);
  public: void SetTab(int tab_index, const TCITEM* tab_data);

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidResize() override;
  private: virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam,
                                     LPARAM lParam) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  private: virtual void OnPaint(const Rect rect) override;

  DISALLOW_COPY_AND_ASSIGN(TabStrip);
};

}   // views

#endif //!defined(INCLUDE_evita_views_tab_strip_h)
