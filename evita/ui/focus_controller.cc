// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "evita/ui/focus_controller.h"

#include "evita/ui/base/selection_state.h"
#include "evita/ui/root_widget.h"
#include "evita/ui/widget.h"

namespace ui {

namespace {

Widget* GetTopLevelWidget(Widget* widget) {
  auto runner = widget;
  while (runner && runner->parent_node() &&
         runner->parent_node() != RootWidget::instance()) {
    runner = runner->parent_node();
  }
  return runner;
}

bool IsPopupWindow(HWND hwnd) {
  while (hwnd) {
    auto const dwStyle = static_cast<DWORD>(::GetWindowLong(hwnd, GWL_STYLE));
    if (dwStyle & WS_POPUP)
      return true;
    if (!(dwStyle & WS_CHILD))
      return false;
    hwnd = ::GetParent(hwnd);
  }
  return false;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FocusController::WidgetUseMap
//
class FocusController::WidgetUseMap final {
  private: std::unordered_map<Widget*, int> map_;
  private: int use_tick_;

  public: WidgetUseMap();
  public: ~WidgetUseMap() = default;

  public: Widget* GetRecentUsedWidget() const;
  public: void UseWidget(Widget* widget);
  public: void WillDestroyWidget(Widget* widget);

  DISALLOW_COPY_AND_ASSIGN(WidgetUseMap);
};

FocusController::WidgetUseMap::WidgetUseMap() : use_tick_(0) {
}

Widget* FocusController::WidgetUseMap::GetRecentUsedWidget() const {
  auto widget = static_cast<Widget*>(nullptr);
  auto use_tick = 0;
  for (auto entry : map_) {
    if (!widget || use_tick < entry.second) {
      widget = entry.first;
      use_tick = entry.second;
    }
  }
  return widget;
}

void FocusController::WidgetUseMap::UseWidget(Widget* widget) {
  ++use_tick_;
  auto const it = map_.find(widget);
  if (it == map_.end()) {
    map_[widget] = use_tick_;
    return;
  }
  it->second = use_tick_;
}

void FocusController::WidgetUseMap::WillDestroyWidget(Widget* widget) {
  auto const it = map_.find(widget);
  if (it == map_.end())
    return;
  map_.erase(it);
}

//////////////////////////////////////////////////////////////////////
//
// FocusController
//
FocusController::FocusController()
    : focus_widget_(nullptr), has_active_focus_(false),
      widget_use_map_(new WidgetUseMap()), will_focus_widget_(nullptr) {
}

FocusController::~FocusController() {
}

void FocusController::DidKillNativeFocus(Widget*) {
  if (auto widget = focus_widget_) {
    focus_widget_ = nullptr;
    widget->DidKillFocus(will_focus_widget_);
  }
  has_active_focus_ = false;
}

void FocusController::DidSetNativeFocus(Widget* widget) {
  auto const last_focused_widget = focus_widget_;
  focus_widget_ = will_focus_widget_ ? will_focus_widget_ : widget;
  will_focus_widget_ = nullptr;
  focus_widget_->DidSetFocus(last_focused_widget);
  has_active_focus_ = true;
  auto const host_widget = GetTopLevelWidget(focus_widget_);
  // TODO(eval1749) Should we have |Widget::IsPopupWindow()| to avoid checking
  // WS_POPUP?
  if (IsPopupWindow(host_widget->AssociatedHwnd()))
    return;
  widget_use_map_->UseWidget(host_widget);
}

Widget* FocusController::GetRecentUsedWidget() const {
  return widget_use_map_->GetRecentUsedWidget();
}

SelectionState FocusController::GetSelectionState(Widget* widget) const {
  if (widget->has_focus())
    return SelectionState::HasFocus;

  if (!IsPopupWindow(::GetFocus()))
    return SelectionState::Disabled;

  auto const recent_used_widget = widget_use_map_->GetRecentUsedWidget();
  if (GetTopLevelWidget(widget) == recent_used_widget)
      return SelectionState::Highlight;

  return SelectionState::Disabled;
}

void FocusController::RequestFocus(Widget* widget) {
  DCHECK(!will_focus_widget_);
  // This widget might be hidden during creating window.
  auto host = widget->GetHostWidget();
  if (::GetFocus() == *host->native_window()) {
    if (focus_widget_ == widget)
      return;
    auto const last_focus_widget = focus_widget_;
    focus_widget_ = widget;
    if (last_focus_widget)
      last_focus_widget->DidKillFocus(widget);
    focus_widget_->DidSetFocus(last_focus_widget);
    return;
  }
  will_focus_widget_ = widget;
  ::SetFocus(*host->native_window());
}

void FocusController::WillDestroyWidget(Widget* widget) {
  widget_use_map_->WillDestroyWidget(widget);
  if (focus_widget_ != widget)
    return;
  focus_widget_ = nullptr;
  widget->DidKillFocus(nullptr);
}

}  // namespace ui
