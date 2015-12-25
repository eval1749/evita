// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tabs/tab_strip.h"

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
#include "evita/gfx/factory_set.h"
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
#include "evita/views/tabs/tab_content.h"
#include "evita/views/tabs/tab_strip_delegate.h"
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
auto const kTabHeight = 28;  // = SM_CYCAPTION + SM_CYEDGE + 1

//////////////////////////////////////////////////////////////////////
//
// DragTabCursor
//
class DragTabCursor final : public common::Singleton<DragTabCursor> {
  DECLARE_SINGLETON_CLASS(DragTabCursor);

 public:
  ~DragTabCursor() final;

  HCURSOR GetCursor();

 private:
  DragTabCursor();

  HCURSOR cursor_;

  DISALLOW_COPY_AND_ASSIGN(DragTabCursor);
};

DragTabCursor::DragTabCursor() : cursor_(nullptr) {}

DragTabCursor::~DragTabCursor() {}

HCURSOR DragTabCursor::GetCursor() {
  if (cursor_)
    return cursor_;

  cursor_ = ::LoadCursor(nullptr, IDC_ARROW);

  auto const hDll =
      ::LoadLibraryEx(L"ieframe.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE);
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
 public:
  virtual void DidDeleteTab(Tab* tab) = 0;
  virtual void DidInsertTab(Tab* tab) = 0;

 protected:
  ModelObserver() = default;
  virtual ~ModelObserver() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ModelObserver);
};

//////////////////////////////////////////////////////////////////////
//
// ModelDelegate
//
class ModelDelegate {
 public:
  virtual void AddObserver(ModelObserver* observer) = 0;
  virtual Tab::HitTestResult HitTest(const gfx::PointF& point) = 0;
  virtual void InsertBefore(Tab* new_tab, Tab* ref_tab) = 0;
  virtual void RemoveObserver(ModelObserver* observer) = 0;

 protected:
  ModelDelegate() = default;
  virtual ~ModelDelegate() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ModelDelegate);
};

//////////////////////////////////////////////////////////////////////
//
// DragController
//
class DragController final : private ModelObserver {
 public:
  DragController(ui::Widget* owner,
                 ModelDelegate* model,
                 TabController* tab_controller);
  ~DragController();

  void ContinueDragging(const gfx::Point& point);
  void EndDragging(const gfx::Point& point);
  void MaybeStartDrag(Tab* tab, const gfx::Point& location);
  void StopDragging();

 private:
  enum class State {
    Normal,
    Dragging,
    Pending,
  };

  // ModelObserver
  void DidDeleteTab(Tab* tab) override;
  void DidInsertTab(Tab* tab) override;

  State state_;
  gfx::Point drag_start_point_;
  Tab* dragging_tab_;
  ModelDelegate* const model_;
  ui::Widget* const owner_;
  TabController* const tab_controller_;

  DISALLOW_COPY_AND_ASSIGN(DragController);
};

DragController::DragController(ui::Widget* owner,
                               ModelDelegate* model,
                               TabController* tab_controller)
    : dragging_tab_(nullptr),
      model_(model),
      owner_(owner),
      state_(State::Normal),
      tab_controller_(tab_controller) {
  model_->AddObserver(this);
}

DragController::~DragController() {
  model_->RemoveObserver(this);
}

void DragController::ContinueDragging(const gfx::Point& point) {
  if (!dragging_tab_)
    return;
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

void DragController::EndDragging(const gfx::Point& point) {
  if (!dragging_tab_)
    return;
  auto const dragging_tab = dragging_tab_;
  StopDragging();

  auto const result = model_->HitTest(gfx::PointF(point));
  if (!result) {
    auto screen_point = owner_->MapToDesktopPoint(point);
    tab_controller_->DidDropTab(dragging_tab, screen_point);
    return;
  }
  model_->InsertBefore(dragging_tab, result.tab());
}

void DragController::MaybeStartDrag(Tab* tab, const gfx::Point& location) {
  dragging_tab_ = tab;
  state_ = State::Pending;
  drag_start_point_ = location;
  owner_->SetCapture();
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
 public:
  explicit TabCollection(TabController* tab_controller);
  ~TabCollection() final;

  const std::vector<Tab*> tabs() const { return tabs_; }

  void DeleteTab(Tab* tab);
  Tab* GetSelectedTab() const;
  Tab* GetTab(size_t tab_index) const;
  void InsertTab(TabContent* tab_content, size_t tab_index);
  void MaybeStartDrag(Tab* tab, const gfx::Point& location);
  int NonClientHitTest(const gfx::Point& screen_point);
  void ScrollLeft();
  void ScrollRight();
  void SelectTab(Tab* tab);

 private:
  int GetPreferredTabWidth() const;
  void MakeSelectionVisible();
  void MarkDirty();
  void NotifySelectTab();
  void RenumberTabIndex();
  void UpdateBoundsForAllTabs(int tab_width);
  void UpdateLayout();
  void UpdateTextFont();

  // ModelDelegate
  void AddObserver(ModelObserver* observer) final;
  Tab::HitTestResult HitTest(const gfx::PointF& point) final;
  void InsertBefore(Tab* new_tab, Tab* ref_tab) final;
  void RemoveObserver(ModelObserver* observer) final;

  // ui::SystemMetricsObserver
  void DidChangeIconFont() final;

  // ui::Widget
  void DidChangeBounds() final;
  void DidRealize() final;
  gfx::Size GetPreferredSize() const final;
  void OnDraw(gfx::Canvas* canvas) final;
  void OnMouseMoved(const ui::MouseEvent& event) final;
  void OnMouseReleased(const ui::MouseEvent& event) final;

  base::ObserverList<ModelObserver> observers_;
  DragController drag_controller_;
  bool dirty_;
  // Last bounds used for layout tabs.
  gfx::Rect layout_bounds_;
  Tab* selected_tab_;
  bool should_selected_tab_visible_;
  std::vector<Tab*> tabs_;
  int tabs_origin_;
  TabController* const tab_controller_;
  std::unique_ptr<gfx::TextFormat> text_format_;

  DISALLOW_COPY_AND_ASSIGN(TabCollection);
};

TabCollection::TabCollection(TabController* tab_controller)
    : drag_controller_(this, this, tab_controller),
      dirty_(true),
      selected_tab_(nullptr),
      should_selected_tab_visible_(true),
      tabs_origin_(0),
      tab_controller_(tab_controller) {}

TabCollection::~TabCollection() {}

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
  auto const new_tab =
      new Tab(tab_controller_, tab_content, text_format_.get());
  tabs_.insert(tabs_.begin() + static_cast<ptrdiff_t>(tab_index), new_tab);
  AppendChild(new_tab);
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

void TabCollection::MaybeStartDrag(Tab* tab, const gfx::Point& location) {
  drag_controller_.MaybeStartDrag(tab, location);
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
  tab_controller_->DidSelectTab(selected_tab_);
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
  auto const min_tabs_origin =
      bounds.width() -
      static_cast<int>(tabs_.size() * tabs_.front()->bounds().width());
  auto const new_tabs_origin =
      std::max(tabs_origin_ - kScrollWidth, min_tabs_origin);
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
    if (!tab->is_realized())
      tab->RealizeWidget();
    tab_origin = tab_origin.Offset(tab_width, 0);
    auto const visible_bounds = bounds.Intersect(tab->bounds());
    if (visible_bounds.empty()) {
      tab->Hide();
      continue;
    }
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
      ui::SystemMetrics::instance()->icon_font_family(), kLabelFontSize));
  {
    common::ComPtr<IDWriteInlineObject> inline_object;
    COM_VERIFY(gfx::FactorySet::instance()->dwrite().CreateEllipsisTrimmingSign(
        *text_format_, &inline_object));
    DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
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

Tab::HitTestResult TabCollection::HitTest(const gfx::PointF& point) {
  UpdateLayout();
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
  if (tabs_.empty())
    return;
  {
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, GetContentsBounds());
    ui::Widget::OnDraw(canvas);
  }
  auto const bounds = GetContentsBounds();
  auto const background =
      gfx::RectF(gfx::PointF(static_cast<float>(tabs_.back()->bounds().right()),
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
  drag_controller_.ContinueDragging(event.location());
}

void TabCollection::OnMouseReleased(const ui::MouseEvent& event) {
  if (event.flags() & static_cast<int>(ui::EventFlags::NonClient))
    return;
  drag_controller_.EndDragging(event.location());
  // Close tab when left button is released on tab close mark.
  auto const result = HitTest(gfx::PointF(event.location()));
  if (!result || result.part() != Tab::Part::CloseMark)
    return;
  auto const tab = result.tab();
  tab_controller_->RequestCloseTab(tab);
}

//////////////////////////////////////////////////////////////////////
//
// TabListMenu
//
class TabListMenu final {
 public:
  TabListMenu(ui::Widget* owner, const TabCollection* tab_collection);
  ~TabListMenu();

  void Show();

 private:
  HMENU menu_handle_;
  ui::Widget* const owner_;
  const TabCollection* tab_collection_;

  DISALLOW_COPY_AND_ASSIGN(TabListMenu);
};

TabListMenu::TabListMenu(ui::Widget* owner, const TabCollection* tab_collection)
    : menu_handle_(nullptr), owner_(owner), tab_collection_(tab_collection) {}

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
    auto const flags = tab->is_selected() ? MF_STRING | MF_CHECKED : MF_STRING;
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
                     menu_origin.x(), menu_origin.y(), owner_->AssociatedHwnd(),
                     nullptr);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TabStrip::View class
//
class TabStrip::View final : private ui::ButtonListener,
                             private ModelObserver,
                             private TabController {
 public:
  View(TabStrip* widget, TabStripDelegate* delegate);
  ~View() final;

  void DidBeginAnimationFrame(base::Time time);
  void DidChangeBounds();
  void DidRealize();
  void DeleteTab(size_t tab_index);
  size_t GetNumberOfTabs() const;
  Tab* GetSelectedTab() const;
  Tab* GetTab(size_t tab_index) const;
  void InsertTab(TabContent* tab_content, size_t tab_index);
  int NonClientHitTest(const gfx::Point& screen_point);
  // TODO(eval1749): Once we should revise tooltip handling, we should get rid
  // of |TabStrip::View::OnNotify()|.
  LRESULT OnNotify(NMHDR* nmhder);
  void SelectTab(size_t tab_index);
  void SetTabData(size_t tab_index, const domapi::TabData& tab_data);

 private:
  gfx::Rect ComputeBounds() const;
  void DisableButton(ui::Widget* widget);
  void EnableButton(ui::Widget* widget);
  void MarkDirty();
  void UpdateLayout();

  // ButtonListner
  void DidPressButton(ui::Button* sender, const ui::Event& event) final;
  // ModelObserver
  void DidDeleteTab(Tab* tab) final;
  void DidInsertTab(Tab* tab) final;

  // TabController
  void AddTabAnimation(ui::AnimationGroupMember* member) final;
  void DidChangeTabBounds(Tab* tab) final;
  void DidDropTab(Tab* tab, const gfx::Point& screen_point) final;
  void DidSelectTab(Tab* tab) final;
  void MaybeStartDrag(Tab* tab, const gfx::Point& location) final;
  void RemoveTabAnimation(ui::AnimationGroupMember* member) final;
  void RequestCloseTab(Tab* tab) final;
  void RequestSelectTab(Tab* tab) final;

  ui::AnimationGroup animations_;
  std::unique_ptr<gfx::Canvas> canvas_;
  bool dirty_;
  const std::unique_ptr<ui::ArrowButton> list_button_;
  const std::unique_ptr<ui::ArrowButton> scroll_left_button_;
  const std::unique_ptr<ui::ArrowButton> scroll_right_button_;
  Tab* selected_tab_;
  bool should_selected_tab_visible_;
  const std::unique_ptr<TabCollection> tab_collection_;
  TabListMenu tab_list_menu_;
  TabStripDelegate* tab_strip_delegate_;
  ui::Tooltip tooltip_;
  TabStrip* widget_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

TabStrip::View::View(TabStrip* widget, TabStripDelegate* delegate)
    : dirty_(true),
      list_button_(new ui::ArrowButton(ui::ArrowButton::Direction::Down, this)),
      scroll_left_button_(
          new ui::ArrowButton(ui::ArrowButton::Direction::Left, this)),
      scroll_right_button_(
          new ui::ArrowButton(ui::ArrowButton::Direction::Right, this)),
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

TabStrip::View::~View() {}

gfx::Rect TabStrip::View::ComputeBounds() const {
  auto const contents_bounds =
      gfx::ToEnclosingRect(widget_->GetContentsBounds());
  auto const caption_buttons_width = 45 * 3;
  return gfx::Rect(contents_bounds.origin(),
                   gfx::Size(contents_bounds.width() - caption_buttons_width,
                             contents_bounds.height()));
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
  if (!canvas_->IsReady())
    return widget_->RequestAnimationFrame();
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  widget_->OnDraw(canvas_.get());
}

void TabStrip::View::DidChangeBounds() {
  MarkDirty();
}

void TabStrip::View::DidRealize() {
  tab_collection_->SetBounds(
      gfx::ToEnclosingRect(widget_->GetContentsBounds()));
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
  auto const point = widget_->MapFromDesktopPoint(screen_point);
  if (point.y() < 0)
    return HTCAPTION;
  if (!widget_->GetLocalBounds().Contains(point))
    return HTNOWHERE;
  auto const bounds = ComputeBounds();
  if (!bounds.Contains(point))
    return HTCAPTION;
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
  auto const bounds = ComputeBounds();
  auto const tabs_size =
      static_cast<ui::Widget*>(tab_collection_.get())->GetPreferredSize();
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
  scroll_left_button_->SetBounds(gfx::Point(bounds.origin().x(), button_top),
                                 button_size);
  list_button_->SetBounds(
      gfx::Point(bounds.right() - kArrowButtonWidth, button_top), button_size);
  scroll_right_button_->SetBounds(
      gfx::Point(list_button_->bounds().left() - kArrowButtonWidth, button_top),
      button_size);
  tab_collection_->SetBounds(
      gfx::Point(scroll_left_button_->bounds().right(), bounds.top()),
      gfx::Point(scroll_right_button_->bounds().left(), bounds.bottom()));
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

// TabController
void TabStrip::View::AddTabAnimation(ui::AnimationGroupMember* member) {
  animations_.AddMember(member);
}

void TabStrip::View::DidChangeTabBounds(Tab* tab) {
  const auto tab_bounds =
      tab_collection_->GetLocalBounds().Intersect(tab->bounds());
  // Note: We should pass bounds in HWND's coordinate rather than |TabStrip|
  // coordinate.
  tooltip_.SetToolBounds(
      tab, gfx::Rect(tab_bounds.origin() + tab_collection_->origin() +
                         widget_->origin(),
                     tab_bounds.size()));
}

void TabStrip::View::DidDropTab(Tab* tab, const gfx::Point& screen_point) {
  tab_strip_delegate_->DidDropTab(tab->tab_content(), screen_point);
}

void TabStrip::View::DidSelectTab(Tab* tab) {
  tab_strip_delegate_->DidSelectTab(tab ? tab->tab_index() : -1);
}

void TabStrip::View::MaybeStartDrag(Tab* tab, const gfx::Point& location) {
  tab_collection_->MaybeStartDrag(tab, location);
}

void TabStrip::View::RemoveTabAnimation(ui::AnimationGroupMember* member) {
  animations_.RemoveMember(member);
}

void TabStrip::View::RequestCloseTab(Tab* tab) {
  tab_strip_delegate_->RequestCloseTab(tab->tab_index());
}

void TabStrip::View::RequestSelectTab(Tab* tab) {
  tab_strip_delegate_->RequestSelectTab(tab->tab_index());
}

//////////////////////////////////////////////////////////////////////
//
// TabStrip
//
TabStrip::TabStrip(TabStripDelegate* delegate)
    : view_(new View(this, delegate)) {}

TabStrip::~TabStrip() {}

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
  return view_->NonClientHitTest(screen_point);
}

LRESULT TabStrip::OnNotify(NMHDR* nmhdr) {
  return view_->OnNotify(nmhdr);
}

}  // namespace views
