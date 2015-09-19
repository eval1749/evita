// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_FOCUS_CONTROLLER_H_
#define EVITA_UI_FOCUS_CONTROLLER_H_

#include <memory>

#include "base/basictypes.h"
#include "common/memory/singleton.h"

namespace ui {

enum class SelectionState;
class Widget;

class FocusController final : public common::Singleton<FocusController> {
  DECLARE_SINGLETON_CLASS(FocusController);

 public:
  ~FocusController() final;

  Widget* focus_widget() const { return focus_widget_; }

  // When mode less dialog box has focus, ::GetFocus() returns it, but
  // |have_active_focus_| is false.
  bool has_active_focus() const { return has_active_focus_; }

  // Called when widget, which has native window, loses native focus. This is
  // good time to restore caret background image.
  void DidKillNativeFocus(Widget* widget);

  // Called when widget, which has native window, get native focus. This is
  // good time to prepare caret rendering.
  void DidSetNativeFocus(Widget* widget);
  Widget* GetRecentUsedWidget() const;
  SelectionState GetSelectionState(Widget* widget) const;
  void RequestFocus(Widget* widget);
  void WillDestroyWidget(Widget* widget);

 private:
  class WidgetUseMap;

  FocusController();

  Widget* focus_widget_;
  bool has_active_focus_;
  std::unique_ptr<WidgetUseMap> widget_use_map_;
  Widget* will_focus_widget_;

  DISALLOW_COPY_AND_ASSIGN(FocusController);
};

}  // namespace ui

#endif  // EVITA_UI_FOCUS_CONTROLLER_H_
