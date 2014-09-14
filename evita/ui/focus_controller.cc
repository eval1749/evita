// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/focus_controller.h"

#include "evita/ui/widget.h"

namespace ui {

FocusController::FocusController()
    : focus_widget_(nullptr), has_active_focus_(false),
      last_non_popup_focus_widget_(nullptr), will_focus_widget_(nullptr) {
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
}

void FocusController::RequestFocus(Widget* widget) {
  DCHECK(!will_focus_widget_);
  // This widget might be hidden during creating window.
  auto& host = widget->GetHostWidget();
  if (::GetFocus() == *host.native_window()) {
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
  ::SetFocus(*host.native_window());
}

void FocusController::WillDestroyWidget(Widget* widget) {
  if (focus_widget_ != widget)
    return;
  focus_widget_ = nullptr;
  widget->DidKillFocus(nullptr);
}

}  // namespace ui
