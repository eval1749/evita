// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_strip.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "common/castable.h"
#include "common/memory/singleton.h"
#include "evita/dom/public/tab_data.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/animation/animation_group.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/controls/arrow_button.h"
#include "evita/ui/events/event.h"
#include "evita/ui/system_metrics.h"
#include "evita/ui/system_metrics_observer.h"
#include "evita/ui/tooltip.h"
#include "evita/views/frame_list.h"
#include "evita/views/tabs/tab.h"
#include "evita/views/tab_content.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/vi_Frame.h"

namespace views {
namespace {

//////////////////////////////////////////////////////////////////////
//
// TabStrip Design Parameters
//
enum TabStripDesignParams {
  k_cxMargin = 0,
  k_cxEdge = 2,
  k_cxBorder = 3,
  k_cxPad = 3,
  k_cyBorder = 5,
  k_cyIcon = 16,
};

const auto kArrowButtonWidth = 20;
const auto kArrowButtonHeight = 20;
// Size of small icon.
const auto kIconWidth = 16.0f;
const auto kIconHeight = 16.0f;
// |kLabelFontSize| depends on |kIconHeight|. If you change this value,
// please make sure "g" and "y" characters are fully displayed in a tab.
const auto kLabelFontSize = 13.0f;
const auto kScrollWidth = 10;
// On Win8.1
//  SM_CYSIZE = 22
//  SM_CYCAPTION = 23
//  SM_CYEDGE = 2
//  SM_CYSIZEFRAME = 4
auto const kTabHeight = 28; // SM_CYCAPTION + SM_CYEDGE + 1

//////////////////////////////////////////////////////////////////////
//
// DragTabCursor
//
class DragTabCursor : public common::Singleton<DragTabCursor> {
  DECLARE_SINGLETON_CLASS(DragTabCursor);

  private: HCURSOR cursor_;

  private: DragTabCursor();
  public: virtual ~DragTabCursor();

  public: HCURSOR GetCursor();

  DISALLOW_COPY_AND_ASSIGN(DragTabCursor);
};

DragTabCursor::DragTabCursor() : cursor_(nullptr) {
}

DragTabCursor::~DragTabCursor() {
}

HCURSOR DragTabCursor::GetCursor() {
  if (cursor_)
    return cursor_;

  cursor_ = ::LoadCursor(nullptr, IDC_ARROW);

  auto const hDll = ::LoadLibraryEx(L"ieframe.dll", nullptr,
                                    LOAD_LIBRARY_AS_DATAFILE);
  if (!hDll)
    return cursor_;

  if (auto const hCursor = ::LoadCursor(hDll, MAKEINTRESOURCE(643))) {
    // Note: |CopyCursor()| is macro instead of function.
    if (auto const hCursorCopy = CopyCursor(hCursor))
      cursor_ = hCursorCopy;
  }

  ::FreeLibrary(hDll);

  return cursor_;
}

//////////////////////////////////////////////////////////////////////
//
// ModelObserver
//
class ModelObserver {
  protected: ModelObserver() = default;
  protected: virtual ~ModelObserver() = default;

  public: virtual void DidDeleteTab(Tab* tab) = 0;
  public: virtual void DidInsertTab(Tab* tab) = 0;

  DISALLOW_COPY_AND_ASSIGN(ModelObserver);
};

//////////////////////////////////////////////////////////////////////
//
// ModelDelegate
//
class ModelDelegate {
  protected: ModelDelegate() = default;
  protected: virtual ~ModelDelegate() = default;

  public: virtual void AddObserver(ModelObserver* observer) = 0;
  public: virtual Tab::HitTestResult HitTest(
      const gfx::PointF& point) const = 0;
  public: virtual void InsertBefore(Tab* new_tab, Tab* ref_tab) = 0;
  public: virtual void RemoveObserver(ModelObserver* observer) = 0;

  DISALLOW_COPY_AND_ASSIGN(ModelDelegate);
};

//////////////////////////////////////////////////////////////////////
//
// DragController
//
class DragController final : private ModelObserver {
  private: enum class State {
    Normal,
    Dragging,
    Pending,
  };

  private: State state_;
  private: gfx::Point drag_start_point_;
  private: Tab* dragging_tab_;
  private: ModelDelegate* const model_;
  private: ui::Widget* const owner_;
  private: TabOwner* const view_delegate_;

  public: DragController(ui::Widget* owner,
                         ModelDelegate* model,
                         TabOwner* view_delegate);
  public: ~DragController();

  public: bool is_dragging() const { return dragging_tab_; }

  public: void OnMouseMoved(const ui::MouseEvent& event);
  public: void OnMousePressed(const ui::MouseEvent& event);
  public: void OnMouseReleased(const ui::MouseEvent& event);
  public: void StopDragging();

  // ModelObserver
  private: void DidDeleteTab(Tab* tab) override;
  private: void DidInsertTab(Tab* tab) override;

  DISALLOW_COPY_AND_ASSIGN(DragController);
};

DragController::DragController(ui::Widget* owner, ModelDelegate* model,
                               TabOwner* view_delegate)
    : dragging_tab_(nullptr), model_(model), owner_(owner),
      state_(State::Normal), view_delegate_(view_delegate) {
  model_->AddObserver(this);
}

DragController::~DragController() {
  model_->RemoveObserver(this);
}

void DragController::OnMouseMoved(const ui::MouseEvent& event) {
  if (!dragging_tab_)
    return;
  auto const point = event.location();
  if (state_ == State::Pending) {
    if (point.x() - drag_start_point_.x() >= -5 &&
        point.x() - drag_start_point_.x() <= 5) {
      return;
    }

    state_ = State::Dragging;
  }

  auto const result = model_->HitTest(gfx::PointF(point));
  if (!result)
    return;

  ::SetCursor(DragTabCursor::instance()->GetCursor());
}

void DragController::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  auto const result = model_->HitTest(gfx::PointF(event.location()));
  if (!result || result.part() != Tab::Part::Label)
    return;
  auto const tab = result.tab();
  if (!tab->is_selected())
    view_delegate_->RequestSelectTab(tab);
  dragging_tab_ = tab;
  state_ = State::Pending;
  drag_start_point_ = event.location();
  owner_->SetCapture();
}

void DragController::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button() || state_ == State::Normal)
    return;

  auto const dragging_tab = dragging_tab_;
  StopDragging();

  auto const result = model_->HitTest(gfx::PointF(event.location()));
  if (!result) {
    view_delegate_->DidDropTab(dragging_tab, event.screen_location());
    return;
  }
  model_->InsertBefore(dragging_tab, result.tab());
}

void DragController::StopDragging() {
  state_ = State::Normal;
  dragging_tab_ = nullptr;
  owner_->ReleaseCapture();
  ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

// ModelObserver
void DragController::DidDeleteTab(Tab* tab) {
  if (dragging_tab_ != tab)
    return;
  StopDragging();
}

void DragController::DidInsertTab(Tab* tab) {
  if (dragging_tab_ != tab)
    return;
  StopDragging();
}

//////////////////////////////////////////////////////////////////////
//
// TabCollection
//
class TabCollection final : public ui::Widget,
                            public ModelDelegate,
                            private ui::SystemMetricsObserver {
  private: ObserverList<ModelObserver> observers_;

  private: DragController drag_controller_;
  private: bool dirty_;
  // Last bounds used for layout tabs.
  private: gfx::Rect layout_bounds_;
  private: Tab* selected_tab_;
  private: bool should_selected_tab_visible_;
  private: std::vector<Tab*> tabs_;
  private: int tabs_origin_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;
  private: TabOwner* const view_delegate_;

  public: TabCollection(TabOwner* view_delegate);
  public: virtual ~TabCollection();

  public: const std::vector<Tab*> tabs() const { return tabs_; }

  public: void DeleteTab(Tab* tab);
  private: int GetPreferredTabWidth() const;
  public: Tab* GetSelectedTab() const;
  public: Tab* GetTab(size_t tab_index) const;
  public: void InsertTab(TabContent* tab_content, size_t tab_index);
  private: void MakeSelectionVisible();
  private: void MarkDirty();
  public: int NonClientHitTest(const gfx::Point& screen_point);
  private: void NotifySelectTab();
  private: void RenumberTabIndex();
  public: void ScrollLeft();
  public: void ScrollRight();
  public: void SelectTab(Tab* tab);
  private: void UpdateBoundsForAllTabs(int tab_width);
  private: void UpdateLayout();
  private: void UpdateTextFont();

  // ModelDelegate
  private: void AddObserver(ModelObserver* observer) override;
  private: Tab::HitTestResult HitTest(const gfx::PointF& point) const override;
  private: void InsertBefore(Tab* new_tab, Tab* ref_tab) override;
  private: void RemoveObserver(ModelObserver* observer) override;

  // ui::SystemMetricsObserver
  private: virtual void DidChangeIconFont() override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidRealize() override;
  private: virtual gfx::Size GetPreferredSize() const override;
  private: virtual void OnDraw(gfx::Canvas* canvas) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(TabCollection);
};

TabCollection::TabCollection(TabOwner* view_delegate)
    : drag_controller_(this, this, view_delegate), dirty_(true),
      selected_tab_(nullptr),
      should_selected_tab_visible_(true), tabs_origin_(0),
      view_delegate_(view_delegate) {
}

TabCollection::~TabCollection() {
}

void TabCollection::DeleteTab(Tab* tab) {
  MarkDirty();
  auto selection_changed = false;
  if (tabs_.size() == 1) {
    tabs_.clear();
    selected_tab_ = nullptr;
  } else {
    if (tab == selected_tab_) {
      selection_changed = true;
      selected_tab_ = nullptr;
    }
    auto const it = std::find(tabs_.begin(), tabs_.end(), tab);
    DCHECK(it != tabs_.end());
    tabs_.erase(it);
    RenumberTabIndex();
  }

  tab->DestroyWidget();

  if (selection_changed)
    NotifySelectTab();

  FOR_EACH_OBSERVER(ModelObserver, observers_, DidDeleteTab(tab));
}

int TabCollection::GetPreferredTabWidth() const {
  auto tab_width = 0;
  for (auto const tab : tabs_)
    tab_width = std::max(tab_width, tab->GetPreferredWidth());
  return tab_width;
}

Tab* TabCollection::GetSelectedTab() const {
  return selected_tab_;
}

Tab* TabCollection::GetTab(size_t tab_index) const {
  if (tab_index >= tabs_.size())
    return nullptr;
  return tabs_[tab_index];
}

void TabCollection::InsertTab(TabContent* tab_content, size_t tab_index_in) {
  MarkDirty();
  DCHECK(text_format_);
  auto const tab_index = std::min(tab_index_in, tabs_.size());
  auto const new_tab = new Tab(view_delegate_, tab_content, text_format_.get());
  tabs_.insert(tabs_.begin() + static_cast<ptrdiff_t>(tab_index), new_tab);
  // TODO(eval1749) Should we allow to realize empty bounds widget?
  new_tab->SetBounds(gfx::Rect(gfx::Size(1, 1)));
  AppendChild(new_tab);
  if (is_realized())
    new_tab->RealizeWidget();
  RenumberTabIndex();
  FOR_EACH_OBSERVER(ModelObserver, observers_, DidInsertTab(new_tab));
}

void TabCollection::MakeSelectionVisible() {
  if (!selected_tab_)
    return;
  if (!should_selected_tab_visible_) {
    should_selected_tab_visible_ = true;
    MarkDirty();
    return;
  }
  UpdateLayout();
  if (bounds().Contains(selected_tab_->bounds()))
    return;
  MarkDirty();
}

void TabCollection::MarkDirty() {
  if (dirty_)
    return;
  SchedulePaint();
  dirty_ = true;
}

int TabCollection::NonClientHitTest(const gfx::Point& screen_point) {
  auto const point = MapFromDesktopPoint(screen_point);
  if (!GetLocalBounds().Contains(point))
    return HTNOWHERE;
  auto const result = HitTest(gfx::PointF(point));
  if (!result)
    return HTCAPTION;
  if (result.part() == Tab::Part::CloseMark)
    return HTCLIENT;
  return tabs_.size() == 1 ? HTCAPTION : HTCLIENT;
}

void TabCollection::NotifySelectTab() {
  view_delegate_->DidSelectTab(selected_tab_);
}

void TabCollection::RenumberTabIndex() {
  auto tab_index = 0;
  for (auto const tab : tabs_) {
    tab->set_tab_index(tab_index);
    ++tab_index;
  }
}

void TabCollection::ScrollLeft() {
  auto const new_tabs_origin = std::min(tabs_origin_ + kScrollWidth, 0);
  if (tabs_origin_ == new_tabs_origin)
    return;
  tabs_origin_ = new_tabs_origin;
  should_selected_tab_visible_ = false;
  MarkDirty();
}

void TabCollection::ScrollRight() {
  auto const bounds = gfx::ToEnclosingRect(GetContentsBounds());
  auto const min_tabs_origin = bounds.width() -
      static_cast<int>(tabs_.size() * tabs_.front()->bounds().width());
  auto const new_tabs_origin = std::max(tabs_origin_ - kScrollWidth,
                                        min_tabs_origin);
  if (tabs_origin_ == new_tabs_origin)
    return;
  tabs_origin_ = new_tabs_origin;
  should_selected_tab_visible_ = false;
  MarkDirty();
}

void TabCollection::SelectTab(Tab* tab) {
  DCHECK(tab);
  if (selected_tab_ != tab) {
    if (selected_tab_)
      selected_tab_->Unselect();
    selected_tab_ = tab;
    if (tab)
      tab->Select();
    NotifySelectTab();
  }
  MakeSelectionVisible();
}

void TabCollection::UpdateBoundsForAllTabs(int tab_width) {
  auto const bounds = gfx::ToEnclosingRect(GetContentsBounds());
  auto tab_origin = gfx::Point(tabs_origin_ + bounds.left(), bounds.top());
  auto const tab_size = gfx::Size(tab_width, bounds.bottom() - tab_origin.y());
  for (auto const tab : tabs_) {
    tab->SetBounds(gfx::Rect(tab_origin, tab_size));
    tab_origin = tab_origin.Offset(tab_width, 0);
    // Pass tool bounds in |TabCollection| coordinate.
    auto const visible_bounds = bounds.Intersect(tab->bounds());
    if (visible_bounds.empty()) {
      tab->Hide();
      view_delegate_->SetToolBounds(tab, gfx::Rect());
      continue;
    }
    view_delegate_->SetToolBounds(tab, gfx::Rect(
        visible_bounds.origin() + origin(),
        visible_bounds.size()));
    tab->Show();
  }
}

void TabCollection::UpdateLayout() {
  if (!dirty_)
    return;
  dirty_ = false;
  auto const bounds = gfx::ToEnclosingRect(GetContentsBounds());
  if (tabs_.empty()) {
    tabs_origin_ = 0;
    return;
  }
  auto const tab_width = GetPreferredTabWidth();
  // +1 for last edge
  auto const tabs_width = static_cast<int>(tabs_.size() * tab_width + 1);

  if (layout_bounds_ == bounds) {
    if (tabs_width < bounds.width())
      tabs_origin_ = 0;
    UpdateBoundsForAllTabs(tab_width);
    if (!selected_tab_ || !should_selected_tab_visible_ ||
        bounds.Contains(selected_tab_->bounds())) {
      return;
    }
  } else {
    tabs_origin_ = 0;
    layout_bounds_ = bounds;
    UpdateBoundsForAllTabs(tab_width);
    if (!selected_tab_)
      return;
  }

  // Make selected tab visible.
  auto const last_view_port_left = tabs_width - bounds.width();
  auto const selected_left = selected_tab_->tab_index() * tab_width;
  auto const selected_right = selected_left + tab_width;
  if (selected_right < bounds.width()) {
    // |selected_tab_| is in left most part.
    tabs_origin_ = 0;
  } else if (selected_left >= last_view_port_left) {
    // |selected_tab_| is in right most part.
    tabs_origin_ = -last_view_port_left;
  } else if (selected_tab_->bounds().left() < bounds.left()) {
    // |selected_tab_| is left of view port.
    tabs_origin_ = -selected_left;
  } else {
    // |selected_tab_| is right of view port.
    // -1 for last edge
    tabs_origin_ = -(selected_right - bounds.width() - 1);
  }
  UpdateBoundsForAllTabs(tab_width);
}

void TabCollection::UpdateTextFont() {
  // To fit into icon height, we use |kLabelFontSize| rather than
  // |ui::SystemMetrics::instance()->icon_font_size()|.
  text_format_.reset(new gfx::TextFormat(
      ui::SystemMetrics::instance()->icon_font_family(),
      kLabelFontSize));
  {
    common::ComPtr<IDWriteInlineObject> inline_object;
    COM_VERIFY(gfx::FactorySet::instance()->dwrite().
        CreateEllipsisTrimmingSign(*text_format_, &inline_object));
    DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    (*text_format_)->SetTrimming(&trimming, inline_object);
  }
  (*text_format_)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  for (auto const tab : tabs_)
    tab->SetTextFormat(text_format_.get());
  MarkDirty();
}

// ModelDelegate
void TabCollection::AddObserver(ModelObserver* observer) {
  observers_.AddObserver(observer);
}

Tab::HitTestResult TabCollection::HitTest(const gfx::PointF& point) const {
  for (auto const tab : tabs_) {
    auto const point_in_tab = point - gfx::PointF(tab->origin());
    auto const result = tab->HitTest(point_in_tab);
    if (result)
      return result;
  }
  return Tab::HitTestResult();
}

void TabCollection::InsertBefore(Tab* new_tab, Tab* ref_tab) {
  MarkDirty();
  tabs_.erase(tabs_.begin() + new_tab->tab_index());
  tabs_.insert(tabs_.begin() + ref_tab->tab_index(), new_tab);
  RenumberTabIndex();
  FOR_EACH_OBSERVER(ModelObserver, observers_, DidInsertTab(new_tab));
}

void TabCollection::RemoveObserver(ModelObserver* observer) {
  observers_.RemoveObserver(observer);
}

// ui::SystemMetricsObserver
void TabCollection::DidChangeIconFont() {
  UpdateTextFont();
}

// ui::Widget
void TabCollection::DidChangeBounds() {
  MarkDirty();
  should_selected_tab_visible_ = true;
  // Since canvas is empty, we should paint all tabs.
  for (auto const tab : tabs_)
    tab->MarkDirty();
}

void TabCollection::DidRealize() {
  UpdateTextFont();
}

gfx::Size TabCollection::GetPreferredSize() const {
  auto const tab_width = GetPreferredTabWidth();
  return gfx::Size(static_cast<int>(tab_width * tabs_.size()), kTabHeight);
}

void TabCollection::OnDraw(gfx::Canvas* canvas) {
  UpdateLayout();
  {
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, GetContentsBounds());
    ui::Widget::OnDraw(canvas);
  }
  auto const bounds = GetContentsBounds();
  auto const background = gfx::RectF(
      gfx::PointF(static_cast<float>(tabs_.back()->bounds().right()),
                  bounds.top),
      bounds.bottom_right());
  if (background.empty())
    return;
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, background);
  canvas->AddDirtyRect(background);
  canvas->Clear(gfx::ColorF());
}

void TabCollection::OnMouseMoved(const ui::MouseEvent& event) {
  if (event.flags() & static_cast<int>(ui::EventFlags::NonClient))
    return;
  if (drag_controller_.is_dragging()) {
    drag_controller_.OnMouseMoved(event);
    return;
  }
}

void TabCollection::OnMousePressed(const ui::MouseEvent& event) {
  if (event.flags() & static_cast<int>(ui::EventFlags::NonClient))
    return;
  drag_controller_.OnMousePressed(event);
}

void TabCollection::OnMouseReleased(const ui::MouseEvent& event) {
  if (event.flags() & static_cast<int>(ui::EventFlags::NonClient))
    return;
  if (drag_controller_.is_dragging()) {
    drag_controller_.OnMouseReleased(event);
    return;
  }
  // Close tab when left button is released on tab close mark.
  auto const result = HitTest(gfx::PointF(event.location()));
  if (!result || result.part() != Tab::Part::CloseMark)
    return;
  auto const tab = result.tab();
  view_delegate_->RequestCloseTab(tab);
}

//////////////////////////////////////////////////////////////////////
//
// TabListMenu
//
class TabListMenu final {
  private: HMENU menu_handle_;
  private: ui::Widget* const owner_;
  private: const TabCollection* tab_collection_;

  public: TabListMenu(ui::Widget* owner, const TabCollection* tab_collection);
  public: ~TabListMenu();

  public: void Show();

  DISALLOW_COPY_AND_ASSIGN(TabListMenu);
};

TabListMenu::TabListMenu(ui::Widget* owner, const TabCollection* tab_collection)
    : menu_handle_(nullptr), owner_(owner), tab_collection_(tab_collection) {
}

TabListMenu::~TabListMenu() {
  if (!menu_handle_)
    return;
  ::DestroyMenu(menu_handle_);
}

void TabListMenu::Show() {
  auto const menu_origin = owner_->MapToDesktopPoint(gfx::Point());
  if (!menu_handle_)
    menu_handle_ = ::CreatePopupMenu();

  // Make Tab List Menu empty
  while (::GetMenuItemCount(menu_handle_) > 0)
    ::DeleteMenu(menu_handle_, 0, MF_BYPOSITION);

  // Add Tab name to menu.
  auto last_tab = static_cast<Tab*>(nullptr);
  for (auto const tab : tab_collection_->tabs()) {
    auto const flags = tab->is_selected() ? MF_STRING | MF_CHECKED :
                                            MF_STRING;
    if (last_tab &&
        last_tab->bounds().right() < 0 != tab->bounds().right() < 0) {
      ::AppendMenu(menu_handle_, MF_SEPARATOR, 0, nullptr);
    }
    last_tab = tab;
    ::AppendMenu(menu_handle_, static_cast<DWORD>(flags),
                 static_cast<DWORD>(tab->tab_index()),
                 tab->label_text().c_str());
  }

  ::TrackPopupMenuEx(menu_handle_, TPM_LEFTALIGN | TPM_TOPALIGN, 
      menu_origin.x(), menu_origin.y(), owner_->AssociatedHwnd(), nullptr);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TabStrip::View class
//
class TabStrip::View final : private ui::ButtonListener,
                             private ModelObserver,
                             private TabOwner {
  private: ui::AnimationGroup animations_;
  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: bool dirty_;
  private: const std::unique_ptr<ui::ArrowButton> list_button_;
  private: const std::unique_ptr<ui::ArrowButton> scroll_left_button_;
  private: const std::unique_ptr<ui::ArrowButton> scroll_right_button_;
  private: Tab* selected_tab_;
  private: bool should_selected_tab_visible_;
  private: const std::unique_ptr<TabCollection> tab_collection_;
  private: TabListMenu tab_list_menu_;
  private: TabStripDelegate* tab_strip_delegate_;
  private: ui::Tooltip tooltip_;
  private: TabStrip* widget_;

  public: View(TabStrip* widget, TabStripDelegate* delegate);
  public: virtual ~View();

  public: void DidBeginAnimationFrame(base::Time time);
  public: void DidChangeBounds();
  public: void DidRealize();
  public: void DeleteTab(size_t tab_index);
  private: void DisableButton(ui::Widget* widget);
  private: void EnableButton(ui::Widget* widget);
  public: size_t GetNumberOfTabs() const;
  public: Tab* GetSelectedTab() const;
  public: Tab* GetTab(size_t tab_index) const;
  public: void InsertTab(TabContent* tab_content, size_t tab_index);
  private: void MarkDirty();
  public: int NonClientHitTest(const gfx::Point& screen_point);
  // TODO(eval1749) Once we should revise tooltip handling, we should get rid
  // of |TabStrip::View::OnNotify()|.
  public: LRESULT OnNotify(NMHDR* nmhder);
  public: void SelectTab(size_t tab_index);
  public: void SetTabData(size_t tab_index, const domapi::TabData& tab_data);
  private: void UpdateLayout();

  // ButtonListner
  private: virtual void DidPressButton(ui::Button* sender,
                                       const ui::Event& event) override;
  // ModelObserver
  private: void DidDeleteTab(Tab* tab) override;
  private: void DidInsertTab(Tab* tab) override;

  // TabOwner
  private: virtual void AddAnimation(
      ui::AnimationGroupMember* member) override;
  private: virtual void DidDropTab(Tab* tab,
                                   const gfx::Point& screen_point) override;
  private: virtual void DidSelectTab(Tab* tab) override;
  private: virtual base::string16 GetTooltipTextForTab(Tab* tab) override;
  private: virtual void RemoveAnimation(
      ui::AnimationGroupMember* member) override;
  private: virtual void RequestCloseTab(Tab* tab) override;
  private: virtual void RequestSelectTab(Tab* tab) override;
  private: virtual void SetToolBounds(Tab* tab,
                                      const gfx::Rect& bounds) override;

  DISALLOW_COPY_AND_ASSIGN(View);
};

TabStrip::View::View(TabStrip* widget, TabStripDelegate* delegate)
    : dirty_(true), list_button_(new ui::ArrowButton(
          ui::ArrowButton::Direction::Down, this)),
      scroll_left_button_(new ui::ArrowButton(
          ui::ArrowButton::Direction::Left, this)),
      scroll_right_button_(new ui::ArrowButton(
          ui::ArrowButton::Direction::Right, this)),
      tab_collection_(new TabCollection(this)),
      tab_list_menu_(list_button_.get(), tab_collection_.get()),
      tab_strip_delegate_(delegate),
      widget_(widget) {
  static_cast<ModelDelegate*>(tab_collection_.get())->AddObserver(this);
  list_button_->set_owned_by_client();
  scroll_left_button_->set_owned_by_client();
  scroll_right_button_->set_owned_by_client();
  tab_collection_->set_owned_by_client();
}

TabStrip::View::~View() {
}

void TabStrip::View::DeleteTab(size_t tab_index) {
  auto const tab = tab_collection_->GetTab(static_cast<size_t>(tab_index));
  if (!tab)
    return;
  tab_collection_->DeleteTab(tab);
}

void TabStrip::View::DidBeginAnimationFrame(base::Time time) {
  UpdateLayout();
  if (animations_.Animate(time))
    widget_->RequestAnimationFrame();
  if (!canvas_)
    canvas_.reset(widget_->layer()->CreateCanvas());
  else if (widget_->GetContentsBounds() != canvas_->GetLocalBounds())
    canvas_->SetBounds(widget_->GetContentsBounds());
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  widget_->OnDraw(canvas_.get());
}

void TabStrip::View::DidChangeBounds() {
  MarkDirty();
}

void TabStrip::View::DidRealize() {
  tab_collection_->SetBounds(gfx::ToEnclosingRect(
      widget_->GetContentsBounds()));
  widget_->AppendChild(tab_collection_.get());
  tab_collection_->RealizeWidget();
  tooltip_.Realize(widget_->AssociatedHwnd());
}

void TabStrip::View::DisableButton(ui::Widget* button) {
  if (!button->parent_node())
    return;
  button->Hide();
  widget_->RemoveChild(button);
}

void TabStrip::View::EnableButton(ui::Widget* button) {
  if (!button->parent_node())
    widget_->AppendChild(button);
  if (!button->is_realized())
    button->RealizeWidget();
  if (!button->visible())
    button->Show();
}

size_t TabStrip::View::GetNumberOfTabs() const {
  return tab_collection_->tabs().size();
}

Tab* TabStrip::View::GetSelectedTab() const {
  return tab_collection_->GetSelectedTab();
}

Tab* TabStrip::View::GetTab(size_t tab_index) const {
  return tab_collection_->GetTab(tab_index);
}

void TabStrip::View::InsertTab(TabContent* tab_content, size_t tab_index) {
  tab_collection_->InsertTab(tab_content, tab_index);
}

void TabStrip::View::MarkDirty() {
  if (dirty_)
    return;
  widget_->RequestAnimationFrame();
  dirty_ = true;
}

int TabStrip::View::NonClientHitTest(const gfx::Point& screen_point) {
  UpdateLayout();
  return tab_collection_->NonClientHitTest(screen_point);
}

LRESULT TabStrip::View::OnNotify(NMHDR* nmhdr) {
  tooltip_.OnNotify(nmhdr);
  return 0;
}

void TabStrip::View::SelectTab(size_t tab_index) {
  auto const tab = tab_collection_->GetTab(tab_index);
  if (!tab)
    return;
  tab_collection_->SelectTab(tab);
}

void TabStrip::View::SetTabData(size_t tab_index,
                                const domapi::TabData& tab_data) {
  auto const tab = tab_collection_->GetTab(tab_index);
  if (!tab)
    return;
  tab->SetTabData(tab_data);
}

void TabStrip::View::UpdateLayout() {
  if (!dirty_)
    return;
  dirty_ = false;
  auto const bounds = gfx::ToEnclosingRect(widget_->GetContentsBounds());
  auto const tabs_size = static_cast<ui::Widget*>(tab_collection_.get())->
      GetPreferredSize();
  if (tabs_size.width() <= bounds.width()) {
    // Show only |TabCollection|.
    DisableButton(list_button_.get());
    DisableButton(scroll_left_button_.get());
    DisableButton(scroll_right_button_.get());
    tab_collection_->SetBounds(bounds);
    return;
  }

  // Show |TabCollection| with scroll buttons and list button.
  auto const button_size = gfx::Size(kArrowButtonWidth, bounds.height());
  auto const button_top = bounds.top();
  scroll_left_button_->SetBounds(
      gfx::Point(bounds.origin().x(), button_top), button_size);
  list_button_->SetBounds(
      gfx::Point(bounds.right() - kArrowButtonWidth, button_top), button_size);
  scroll_right_button_->SetBounds(
      gfx::Point(list_button_->bounds().left() - kArrowButtonWidth, button_top),
      button_size);
  tab_collection_->SetBounds(gfx::Point(scroll_left_button_->bounds().right(),
                                        bounds.top()),
                             gfx::Point(scroll_right_button_->bounds().left(),
                                        bounds.bottom()));
  EnableButton(list_button_.get());
  EnableButton(scroll_left_button_.get());
  EnableButton(scroll_right_button_.get());
}

// ButtonListener
void TabStrip::View::DidPressButton(ui::Button* sender, const ui::Event&) {
  if (sender == list_button_.get()) {
    tab_list_menu_.Show();
    return;
  }

  if (sender == scroll_left_button_.get()) {
    tab_collection_->ScrollLeft();
    return;
  }

  if (sender == scroll_right_button_.get()) {
    tab_collection_->ScrollRight();
    return;
  }

  NOTREACHED();
}

// ModelObserver
void TabStrip::View::DidDeleteTab(Tab* tab) {
 tooltip_.DeleteTool(tab);
 MarkDirty();
}

void TabStrip::View::DidInsertTab(Tab* tab) {
 tooltip_.AddTool(tab);
 MarkDirty();
}

// TabOwner
void TabStrip::View::AddAnimation(ui::AnimationGroupMember* member) {
  animations_.AddMember(member);
}

void TabStrip::View::DidDropTab(Tab* tab, const gfx::Point& screen_point) {
  tab_strip_delegate_->DidDropTab(tab->tab_content(), screen_point);
}

void TabStrip::View::DidSelectTab(Tab* tab) {
  tab_strip_delegate_->DidSelectTab(tab ? tab->tab_index() : -1);
}

base::string16 TabStrip::View::GetTooltipTextForTab(Tab* tab) {
  return tab_strip_delegate_->GetTooltipTextForTab(tab->tab_index());
}

void TabStrip::View::RemoveAnimation(ui::AnimationGroupMember* member) {
  animations_.RemoveMember(member);
}

void TabStrip::View::RequestCloseTab(Tab* tab) {
  tab_strip_delegate_->RequestCloseTab(tab->tab_index());
}

void TabStrip::View::RequestSelectTab(Tab* tab) {
  tab_strip_delegate_->RequestSelectTab(tab->tab_index());
}

void TabStrip::View::SetToolBounds(Tab* tab, const gfx::Rect& bounds) {
  // Note: We should pass bounds in HWND's coordinate rather than |TabStrip|
  // coordinate.
  tooltip_.SetToolBounds(tab, gfx::Rect(
    bounds.origin().Offset(widget_->origin().x(), widget_->origin().y()),
    bounds.size()));
}

//////////////////////////////////////////////////////////////////////
//
// TabStrip
//
TabStrip::TabStrip(TabStripDelegate* delegate)
    : view_(new View(this, delegate)) {
}

TabStrip::~TabStrip() {
}

int TabStrip::number_of_tabs() const {
  return static_cast<int>(view_->GetNumberOfTabs());
}

int TabStrip::selected_index() const {
  auto const tab = view_->GetSelectedTab();
  return tab ? tab->tab_index() : -1;
}

void TabStrip::DeleteTab(int tab_index) {
  view_->DeleteTab(static_cast<size_t>(tab_index));
}

TabContent* TabStrip::GetTab(int tab_index) {
  auto const tab = view_->GetTab(static_cast<size_t>(tab_index));
  return tab ? tab->tab_content() : nullptr;
}

void TabStrip::InsertTab(TabContent* tab_content, int new_tab_index) {
  view_->InsertTab(tab_content, static_cast<size_t>(new_tab_index));
}

void TabStrip::SelectTab(int tab_index) {
  view_->SelectTab(static_cast<size_t>(tab_index));
}

void TabStrip::SetTab(int tab_index, const domapi::TabData& tab_data) {
  view_->SetTabData(static_cast<size_t>(tab_index), tab_data);
}

// ui::AnimationFrameHanndler
void TabStrip::DidBeginAnimationFrame(base::Time time) {
  view_->DidBeginAnimationFrame(time);
}

// ui::Widget
void TabStrip::DidChangeBounds() {
  ui::AnimatableWindow::DidChangeBounds();
  view_->DidChangeBounds();
}

void TabStrip::DidRealize() {
  ui::AnimatableWindow::DidRealize();
  SetLayer(new ui::Layer());
  view_->DidRealize();
}

gfx::Size TabStrip::GetPreferredSize() const {
  return Size(300, kTabHeight);
}

int TabStrip::NonClientHitTest(const gfx::Point& screen_point) const {
  auto const point = MapFromDesktopPoint(screen_point);
  if (point.y() < 0)
    return HTCAPTION;
  if (!GetLocalBounds().Contains(point))
    return HTNOWHERE;
  return view_->NonClientHitTest(screen_point);
}

LRESULT TabStrip::OnNotify(NMHDR* nmhdr) {
  return view_->OnNotify(nmhdr);
}

}  // namespace views
