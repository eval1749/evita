// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_view_delegate_impl_h)
#define INCLUDE_evita_view_delegate_impl_h

#include "evita/dom/view_delegate.h"

namespace views {

class ViewDelegateImpl final : public dom::ViewDelegate {
  private: domapi::ViewEventHandler* event_handler_;

  public: ViewDelegateImpl();
  public: ~ViewDelegateImpl() final;

  public: domapi::ViewEventHandler* event_handler() const {
    return event_handler_;
  }

  // ViewDelegate
  private: void AddWindow(dom::WindowId parent_id,
                          dom::WindowId child_id) final;
  private: void ChangeParentWindow(dom::WindowId window_id,
                                   dom::WindowId new_parent_id) final;
  private: text::Posn ComputeOnTextWindow(dom::WindowId window_id,
      const dom::TextWindowCompute& data) final;
  private: void CreateEditorWindow(
      const dom::EditorWindow* window) final;
  private: void CreateFormWindow(dom::WindowId window_id, dom::Form* form,
                                 const domapi::PopupWindowInit& init) final;
  private: void CreateTableWindow(dom::WindowId window_id,
                                  dom::Document* document) final;
  private: void CreateTextWindow(dom::WindowId window_id,
                                 text::Selection* selection) final;
  private: void DestroyWindow(dom::WindowId window_id) final;
  private: void DidStartScriptHost(domapi::ScriptHostState state) final;
  private: void FocusWindow(dom::WindowId window_id) final;
  private: void GetFileNameForLoad(dom::WindowId window_id,
                                   const base::string16& dir_path,
      const GetFileNameForLoadResolver& resolver) final;
  private: void GetFileNameForSave(dom::WindowId window_id,
                                   const base::string16& dir_path,
      const GetFileNameForSaveResolver& resolver) final;
  private: base::string16 GetMetrics(const base::string16& name) final;
  private: domapi::SwitchValue GetSwitch(const base::string16& name) final;
  private: std::vector<base::string16> GetSwitchNames() final;
  private: std::vector<int> GetTableRowStates(dom::WindowId window_id,
      const std::vector<base::string16>& keys) final;
  private: void HideWindow(dom::WindowId window_id) final;
  private: domapi::FloatRect HitTestTextPosition(dom::WindowId window_id,
                                                 text::Posn position);
  private: void MakeSelectionVisible(dom::WindowId window_id) final;
  private: text::Posn MapPointToPosition(domapi::EventTargetId event_target_id,
                                         float x, float y) final;
  private: void MessageBox(dom::WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      const MessageBoxResolver& resolver) final;
  public: void Reconvert(dom::WindowId window_id,
                         const base::string16& text) final;
  private: void RealizeWindow(dom::WindowId window_id) final;
  private: void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) final;
  private: void ReleaseCapture(domapi::EventTargetId event_target_id) final;
  private: void ScrollTextWindow(dom::WindowId window_id, int direction) final;
  private: void SetCapture(domapi::EventTargetId event_target_id) final;
  private: void SetStatusBar(dom::WindowId window_id,
      const std::vector<base::string16>& texts) final;
  private: void SetSwitch(const base::string16& name,
                          const domapi::SwitchValue& new_value) final;
  private: void SetTabData(dom::WindowId window_id,
                                   const domapi::TabData& tab_data) final;
  private: void SetTextWindowZoom(dom::WindowId window_id, float zoom) final;
  private: void ShowWindow(dom::WindowId window_id) final;
  private: void SplitHorizontally(dom::WindowId left_window,
                                  dom::WindowId new_right_window) final;
  private: void SplitVertically(dom::WindowId above_window,
                                dom::WindowId new_below_window) final;
  private: void UpdateWindow(dom::WindowId window_id) final;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegateImpl);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_view_delegate_impl_h)
