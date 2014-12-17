// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_view_delegate_impl_h)
#define INCLUDE_evita_view_delegate_impl_h

#include "evita/dom/view_delegate.h"

namespace views {

class ViewDelegateImpl : public dom::ViewDelegate {
  private: domapi::ViewEventHandler* event_handler_;

  public: ViewDelegateImpl();
  public: virtual ~ViewDelegateImpl();

  public: domapi::ViewEventHandler* event_handler() const {
    return event_handler_;
  }

  // ViewDelegate
  private: virtual void AddWindow(dom::WindowId parent_id,
                                  dom::WindowId child_id) override;
  private: virtual void ChangeParentWindow(dom::WindowId window_id,
      dom::WindowId new_parent_id) override;
  private: virtual text::Posn ComputeOnTextWindow(dom::WindowId window_id,
      const dom::TextWindowCompute& data) override;
  private: virtual void CreateEditorWindow(
      const dom::EditorWindow* window) override;
  private: virtual void CreateFormWindow(
      dom::WindowId window_id, dom::Form* form,
      const domapi::PopupWindowInit& init) override;
  private: virtual void CreateTableWindow(
      dom::WindowId window_id, dom::Document* document) override;
  private: virtual void CreateTextWindow(
      dom::WindowId window_id, text::Selection* selection) override;
  private: virtual void DestroyWindow(dom::WindowId window_id) override;
  private: virtual void DidStartScriptHost(
      domapi::ScriptHostState state) override;
  private: virtual void FocusWindow(dom::WindowId window_id) override;
  private: virtual void GetFileNameForLoad(
      dom::WindowId window_id, const base::string16& dir_path,
      const GetFileNameForLoadResolver& resolver) override;
  private: virtual void GetFileNameForSave(
      dom::WindowId window_id, const base::string16& dir_path,
      const GetFileNameForSaveResolver& resolver) override;
  private: virtual base::string16 GetMetrics(
      const base::string16& name) override;
  private: virtual domapi::SwitchValue GetSwitch(
      const base::string16& name) override;
  private: virtual std::vector<base::string16> GetSwitchNames() override;
  private: virtual std::vector<int> GetTableRowStates(
      dom::WindowId window_id,
      const std::vector<base::string16>& keys) override;
  private: virtual void HideWindow(dom::WindowId window_id) override;
  private: virtual domapi::FloatRect HitTestTextPosition(
      dom::WindowId window_id, text::Posn position);
  private: virtual void MakeSelectionVisible(dom::WindowId window_id) override;
  private: virtual text::Posn MapPointToPosition(
      domapi::EventTargetId event_target_id, float x, float y) override;
  private: virtual void MessageBox(dom::WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      const MessageBoxResolver& resolver) override;
  public: virtual void Reconvert(dom::WindowId window_id,
                                 const base::string16& text) override;
  private: virtual void RealizeWindow(dom::WindowId window_id) override;
  private: virtual void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) override;
  private: virtual void ReleaseCapture(
      domapi::EventTargetId event_target_id) override;
  private: virtual void ScrollTextWindow(dom::WindowId window_id,
                                         int direction) override;
  private: virtual void SetCapture(
      domapi::EventTargetId event_target_id) override;
  private: virtual void SetStatusBar(dom::WindowId window_id,
      const std::vector<base::string16>& texts) override;
  private: virtual void SetSwitch(
      const base::string16& name,
      const domapi::SwitchValue& new_value) override;
  private: virtual void SetTabData(dom::WindowId window_id,
                                   const domapi::TabData& tab_data) override;
  private: virtual void SetTextWindowZoom(dom::WindowId window_id,
                                         float zoom) override;
  private: virtual void ShowWindow(dom::WindowId window_id) override;
  private: virtual void SplitHorizontally(dom::WindowId left_window,
      dom::WindowId new_right_window) override;
  private: virtual void SplitVertically(dom::WindowId above_window,
      dom::WindowId new_below_window) override;
  private: virtual void UpdateWindow(dom::WindowId window_id) override;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegateImpl);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_view_delegate_impl_h)
