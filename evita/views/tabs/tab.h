// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_H_
#define EVITA_VIEWS_TABS_TAB_H_

#include <memory>

#include "evita/dom/public/tab_data.h"
#include "evita/gfx/rect_f.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/animation/animation_group_member.h"
#include "evita/ui/tooltip.h"
#include "evita/ui/widget.h"

namespace gfx {
class TextLayout;
}

namespace ui {
class AnimationFloat;
}

namespace views {

class Tab;
class TabContent;

//////////////////////////////////////////////////////////////////////
//
// TabController
//
class TabController {
 public:
  virtual ~TabController();

  virtual void AddTabAnimation(ui::AnimationGroupMember* member) = 0;
  virtual void DidChangeTabBounds(Tab* tab) = 0;
  virtual void DidDropTab(Tab* tab, const gfx::Point& screen_point) = 0;
  virtual void DidSelectTab(Tab* tab) = 0;
  virtual void MaybeStartDrag(Tab* tab, const gfx::Point& location) = 0;
  virtual void RemoveTabAnimation(ui::AnimationGroupMember* member) = 0;
  virtual void RequestCloseTab(Tab* tab) = 0;
  virtual void RequestSelectTab(Tab* tab) = 0;

 protected:
  TabController();

 private:
  DISALLOW_COPY_AND_ASSIGN(TabController);
};

//////////////////////////////////////////////////////////////////////
//
// Tab
//  Represents a tab.
//
class Tab final : public ui::Widget,
                  private ui::AnimationGroupMember,
                  public ui::Tooltip::ToolDelegate {
 public:
  enum class Part {
    None,
    CloseMark,
    Label,
  };

  class HitTestResult final {
   public:
    HitTestResult(Tab* tab, Part part);
    HitTestResult(const HitTestResult& other);
    HitTestResult();
    ~HitTestResult() = default;

    operator bool() const { return tab_ != nullptr; }

    HitTestResult& operator=(const HitTestResult& other);

    bool operator==(const HitTestResult& other) const;
    bool operator!=(const HitTestResult& other) const;

    Part part() const { return part_; }
    Tab* tab() const { return tab_; }

   private:
    Part part_;
    Tab* tab_;
  };

  enum class State {
    Normal,
    Hovered,
    Selected,
  };

  Tab(TabController* tab_controller,
      TabContent* tab_content,
      gfx::TextFormat* text_format);
  ~Tab() final;

  bool is_selected() const { return state_ == State::Selected; }
  const base::string16& label_text() const { return label_text_; }
  TabContent* tab_content() const { return tab_content_; }
  int tab_index() const { return tab_index_; }

  void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  int GetPreferredWidth() const;
  State GetState(Part part) const;
  HitTestResult HitTest(const gfx::PointF& point);
  void MarkDirty();
  void Select();
  void SetTabData(const domapi::TabData& tab_data);
  void SetTextFormat(gfx::TextFormat* text_format_);
  void Unselect();

 private:
  static float ComputeAlpha(State state);
  gfx::ColorF ComputeBackgroundColor() const;
  void DrawCloseMark(gfx::Canvas* canvas) const;
  void DrawLabel(gfx::Canvas* canvas) const;
  void DrawIcon(gfx::Canvas* canvas) const;
  void DrawTabDataState(gfx::Canvas* canvas) const;
  void SetCloseMarkState(State new_state);
  void SetLabelState(State new_state);
  void SetState(Part part, State new_state);
  void UpdateLayout();

  // ui::AnimationGroupMember
  void Animate(const base::TimeTicks& time) final;

  // ui::Tooltip::ToolDelegate
  base::string16 GetTooltipText() final;

  // ui::Widget
  void DidChangeBounds() final;
  void OnDraw(gfx::Canvas* canvas) final;
  void OnMouseEntered(const ui::MouseEvent& event) final;
  void OnMouseExited(const ui::MouseEvent& event) final;
  void OnMouseMoved(const ui::MouseEvent& event) final;
  void OnMousePressed(const ui::MouseEvent& event) final;
  void OnMouseReleased(const ui::MouseEvent& event) final;

  float animated_alpha_;
  std::unique_ptr<ui::AnimationFloat> animation_alpha_;
  gfx::RectF close_mark_bounds_;
  State close_mark_state_;
  bool dirty_visual_;
  bool dirty_layout_;
  int image_index_;
  gfx::RectF icon_bounds_;
  gfx::RectF label_bounds_;
  base::string16 label_text_;
  State state_;
  TabContent* const tab_content_;
  TabController* const tab_controller_;
  domapi::TabData::State tab_data_state_;
  int tab_index_;
  gfx::TextFormat* text_format_;
  std::unique_ptr<gfx::TextLayout> text_layout_;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_H_
