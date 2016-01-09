// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_VIEW_THREAD_PROXY_H_
#define EVITA_VIEWS_VIEW_THREAD_PROXY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "evita/dom/public/view_delegate.h"

namespace base {
class MessageLoop;
class WaitableEvent;
}

namespace views {

class ViewDelegateImpl;

class ViewThreadProxy : public domapi::ViewDelegate {
 public:
  explicit ViewThreadProxy(base::MessageLoop* message_loop);
  ~ViewThreadProxy() final;

 private:
  // domapi::ViewDelegate
  void AddWindow(domapi::WindowId parent_id, domapi::WindowId child_id) final;
  void ChangeParentWindow(domapi::WindowId window_id,
                          domapi::WindowId new_parent_id) final;
  text::Offset ComputeOnTextWindow(domapi::WindowId window_id,
                                   const domapi::TextWindowCompute& data) final;
  void CreateEditorWindow(domapi::WindowId window_id) final;
  void CreateFormWindow(domapi::WindowId window_id,
                        dom::Form* form,
                        const domapi::PopupWindowInit& init) final;
  void CreateTableWindow(domapi::WindowId window_id,
                         dom::TextDocument* document) final;
  void CreateTextWindow(domapi::WindowId window_id,
                        text::Selection* selection) final;
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
  std::vector<int> GetTableRowStates(
      domapi::WindowId window_id,
      const std::vector<base::string16>& keys) final;
  void HideWindow(domapi::WindowId window_id) final;
  domapi::FloatRect HitTestTextPosition(domapi::WindowId window_id,
                                        text::Offset position);
  void MakeSelectionVisible(domapi::WindowId window_id) final;
  void MapTextFieldPointToOffset(domapi::EventTargetId event_target_id,
                                 float x,
                                 float y,
                                 const domapi::IntegerPromise& promise) final;
  void MapTextWindowPointToOffset(domapi::EventTargetId event_target_id,
                                  float x,
                                  float y,
                                  const domapi::IntegerPromise& promise) final;
  void MessageBox(domapi::WindowId window_id,
                  const base::string16& message,
                  const base::string16& title,
                  int flags,
                  const MessageBoxResolver& resolver) final;
  void Reconvert(domapi::WindowId window_id, const base::string16& text) final;
  void RealizeWindow(domapi::WindowId window_id) final;
  void ReleaseCapture(domapi::EventTargetId event_target_id) final;
  void ScrollTextWindow(domapi::WindowId window_id, int direction) final;
  void SetCapture(domapi::EventTargetId event_target_id) final;
  void SetStatusBar(domapi::WindowId window_id,
                    const std::vector<base::string16>& texts) final;
  void SetSwitch(const base::string16& name,
                 const domapi::SwitchValue& new_value) final;
  void SetTabData(domapi::WindowId window_id,
                  const domapi::TabData& tab_data) final;
  void SetTextWindowZoom(domapi::WindowId window_id, float zoom) final;
  void ShowWindow(domapi::WindowId window_id) final;
  void SplitHorizontally(domapi::WindowId left_window,
                         domapi::WindowId new_right_window) final;
  void SplitVertically(domapi::WindowId above_window,
                       domapi::WindowId new_below_window) final;

  void StartTraceLog(const std::string& config) final;
  void StopTraceLog(const domapi::TraceLogOutputCallback& callback) final;

  void UpdateWindow(domapi::WindowId window_id) final;

  const std::unique_ptr<ViewDelegate> delegate_;
  base::MessageLoop* const message_loop_;
  std::unique_ptr<base::WaitableEvent> waitable_event_;

  DISALLOW_COPY_AND_ASSIGN(ViewThreadProxy);
};

}  // namespace views

#endif  // EVITA_VIEWS_VIEW_THREAD_PROXY_H_
