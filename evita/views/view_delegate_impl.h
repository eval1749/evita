// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_VIEW_DELEGATE_IMPL_H_
#define EVITA_VIEWS_VIEW_DELEGATE_IMPL_H_

#include <string>
#include <vector>

#include "evita/dom/public/view_delegate.h"

namespace views {

class ViewDelegateImpl final : public domapi::ViewDelegate {
 public:
  ViewDelegateImpl();
  ~ViewDelegateImpl() final;

 private:
  // ViewDelegate
  void AddWindow(domapi::WindowId parent_id, domapi::WindowId child_id) final;
  void ChangeParentWindow(domapi::WindowId window_id,
                          domapi::WindowId new_parent_id) final;
  void CreateEditorWindow(domapi::WindowId window_id) final;
  void CreateFormWindow(domapi::WindowId window_id,
                        domapi::WindowId owner_window_id,
                        const domapi::IntRect& bounds,
                        const base::string16& title) final;
  void CreateTextWindow(domapi::WindowId window_id) final;
  void CreateVisualWindow(domapi::WindowId window_id) final;
  void DestroyWindow(domapi::WindowId window_id) final;
  void DidStartScriptHost(domapi::ScriptHostState state) final;
  void DidUpdateDom() final;
  void FocusWindow(domapi::WindowId window_id) final;
  void GetFileNameForLoad(domapi::WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForLoadResolver& resolver) final;
  void GetFileNameForSave(domapi::WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForSaveResolver& resolver) final;
  void GetMetrics(const base::string16& name,
                  const domapi::StringPromise& promise) final;
  domapi::SwitchValue GetSwitch(const base::string16& name) final;
  std::vector<base::string16> GetSwitchNames() final;
  void HideWindow(domapi::WindowId window_id) final;
#undef MessageBox
  void MessageBox(domapi::WindowId window_id,
                  const base::string16& message,
                  const base::string16& title,
                  int flags,
                  const MessageBoxResolver& resolver) final;
  void PaintForm(domapi::WindowId window_id,
                 std::unique_ptr<domapi::Form> form) final;
  void PaintTextArea(
      domapi::WindowId window_id,
      std::unique_ptr<domapi::TextAreaDisplayItem> display_item) final;
  void PaintVisualDocument(
      domapi::WindowId window_id,
      std::unique_ptr<visuals::DisplayItemList> display_item_list) final;
  void Reconvert(domapi::WindowId window_id, const base::string16& text) final;
  void RealizeWindow(domapi::WindowId window_id) final;
  void ReleaseCapture(domapi::EventTargetId event_target_id) final;
  void SetCapture(domapi::EventTargetId event_target_id) final;
  void SetStatusBar(domapi::WindowId window_id,
                    const std::vector<base::string16>& texts) final;
  void SetSwitch(const base::string16& name,
                 const domapi::SwitchValue& new_value) final;
  void SetTabData(domapi::WindowId window_id,
                  const domapi::TabData& tab_data) final;
  void ShowWindow(domapi::WindowId window_id) final;
  void SplitHorizontally(domapi::WindowId left_window,
                         domapi::WindowId new_right_window) final;
  void SplitVertically(domapi::WindowId above_window,
                       domapi::WindowId new_below_window) final;

  void StartTraceLog(const std::string& config) final;
  void StopTraceLog(const domapi::TraceLogOutputCallback& callback) final;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegateImpl);
};

}  // namespace views

#endif  // EVITA_VIEWS_VIEW_DELEGATE_IMPL_H_
