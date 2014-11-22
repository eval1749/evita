// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tabs_tab_h)
#define INCLUDE_evita_views_tabs_tab_h

#include "evita/dom/public/tab_data.h"
#include "evita/gfx/rect_f.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/tooltip.h"

namespace gfx {
class TextLayout;
}

namespace views {

class Tab;
class TabContent;

//////////////////////////////////////////////////////////////////////
//
// ViewDelegate
//
class ViewDelegate {
  protected: ViewDelegate() = default;
  protected: virtual ~ViewDelegate() = default;

  public: virtual void DidDropTab(Tab* tab, const gfx::Point& screen_point) = 0;
  public: virtual void DidSelectTab(Tab* tab) = 0;
  public: virtual base::string16 GetTooltipTextForTab(Tab* tab) = 0;
  public: virtual void RequestAnimationFrame() = 0;
  public: virtual void RequestCloseTab(Tab* tab) = 0;
  public: virtual void RequestSelectTab(Tab* tab) = 0;
  public: virtual void SetToolBounds(Tab* tab, const gfx::Rect& bounds) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

//////////////////////////////////////////////////////////////////////
//
// Tab
//  Represents a tab.
//
class Tab final : public ui::Tooltip::ToolDelegate {
  public: enum class Part {
    None,
    CloseMark,
    Label,
  };

  public: class HitTestResult final {
    private: Part part_;
    private: Tab* tab_;

    public: HitTestResult(Tab* tab, Part part);
    public: HitTestResult(const HitTestResult& other);
    public: HitTestResult();
    public: ~HitTestResult() = default;

    public: operator bool() const { return tab_; }

    public: HitTestResult& operator=(const HitTestResult& other);

    public: bool operator==(const HitTestResult& other) const;
    public: bool operator!=(const HitTestResult& other) const;

    public: Part part() const { return part_; }
    public: Tab* tab() const { return tab_; }
  };

  public: enum class State {
    Normal,
    Hovered,
    Selected,
  };

  private: gfx::RectF bounds_;
  private: gfx::RectF close_mark_bounds_;
  private: State close_mark_state_;
  private: bool dirty_visual_;
  private: bool dirty_layout_;
  private: int image_index_;
  private: gfx::RectF icon_bounds_;
  private: gfx::RectF label_bounds_;
  private: base::string16 label_text_;
  private: State state_;
  private: TabContent* const tab_content_;
  private: domapi::TabData::State tab_data_state_;
  private: int tab_index_;
  private: gfx::TextFormat* text_format_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;
  private: ViewDelegate* const view_delegate_;

  public: Tab(ViewDelegate* view_delegate, TabContent* tab_content,
              gfx::TextFormat* text_format);
  public: virtual ~Tab() = default;

  public: float bottom() const { return bounds_.bottom; }
  public: const gfx::RectF bounds() const { return bounds_; }
  public: bool is_selected() const { return state_ == State::Selected; }
  public: const base::string16& label_text() const { return label_text_; }
  public: float left() const { return bounds_.left; }
  public: float right() const { return bounds_.right; }
  public: int tab_index() const { return tab_index_; }
  public: float top() const { return bounds_.top; }
  public: TabContent* tab_content() const { return tab_content_; }

  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  private: gfx::ColorF ComputeBackgroundColor() const;
  public: void Draw(gfx::Canvas* canvas, const gfx::RectF& content_bounds);
  private: void DrawCloseMark(gfx::Canvas* canvas) const;
  private: void DrawLabel(gfx::Canvas* canvas) const;
  private: void DrawIcon(gfx::Canvas* canvas) const;
  private: void DrawTabDataState(gfx::Canvas* canvas) const;
  public: int GetPreferredWidth() const;
  public: State GetState(Part part) const;
  public: HitTestResult HitTest(const gfx::PointF& point);
  public: void MarkDirty();
  public: void SetBounds(const gfx::RectF& new_bounds);
  private: void SetCloseMarkState(State new_state);
  private: void SetLabelState(State new_state);
  public: void SetState(Part part, State new_state);
  public: void SetTabData(const domapi::TabData& tab_data);
  public: void SetTextFormat(gfx::TextFormat* text_format_);
  private: void UpdateLayout();

  // ui::Tooltip::ToolDelegate
  private: base::string16 GetTooltipText() override;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

}   // views

#endif // !defined(INCLUDE_evita_views_tabs_tab_h)
