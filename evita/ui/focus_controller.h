// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_focus_controller)
#define INCLUDE_evita_ui_focus_controller

#include "base/basictypes.h"
#include "common/memory/singleton.h"

namespace ui {

enum class SelectionState;
class Widget;

class FocusController final : public common::Singleton<FocusController> {
  DECLARE_SINGLETON_CLASS(FocusController);

  private: Widget* focus_widget_;
  private: bool has_active_focus_;
  private: Widget* last_focus_widget_;
  private: Widget* will_focus_widget_;

  private: FocusController();
  public: virtual ~FocusController();

  public: Widget* focus_widget() const { return focus_widget_; }

  // When mode less dialog box has focus, ::GetFocus() returns it, but
  // |have_active_focus_| is false.
  public: bool has_active_focus() const { return has_active_focus_; }

  // Called when widget, which has native window, loses native focus. This is
  // good time to restore caret background image.
  public: void DidKillNativeFocus(Widget* widget);

  // Called when widget, which has native window, get native focus. This is
  // good time to prepare caret rendering.
  public: void DidSetNativeFocus(Widget* widget);
  public: SelectionState GetSelectionState(Widget* widget) const;
  public: void RequestFocus(Widget* widget);
  public: void WillDestroyWidget(Widget* widget);

  DISALLOW_COPY_AND_ASSIGN(FocusController);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_focus_controller)
