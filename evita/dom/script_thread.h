// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCRIPT_THREAD_H_
#define EVITA_DOM_SCRIPT_THREAD_H_

#include <memory>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/dom/view_delegate.h"

namespace base {
class MessageLoop;
class Thread;
class WaitableEvent;
}

namespace dom {

class ScriptHost;

// TODO(eval1749) We should introduce |ViewThreadProxy| class to encapsulate
// inter thread communication rather than in |ScriptThread|.
class ScriptThread final : public ViewDelegate,
                           public domapi::ViewEventHandler {
 public:
  ScriptThread(ViewDelegate* view_delegate, domapi::IoDelegate* io_delegate);
  ~ScriptThread() final;

  void Start(base::MessageLoop* view_message_loop);

 private:
  domapi::ViewEventHandler* view_event_handler() const;

  // ViewDelegate
  void AddWindow(WindowId parent_id, WindowId child_id) final;
  void ChangeParentWindow(WindowId window_id,
                          WindowId new_parent_window_id) final;
  text::Posn ComputeOnTextWindow(WindowId window_id,
                                 const TextWindowCompute& data) final;
  void CreateEditorWindow(const EditorWindow* window) final;
  void CreateFormWindow(WindowId window_id,
                        Form* form,
                        const domapi::PopupWindowInit& init) final;
  void CreateTableWindow(WindowId window_id, Document* document) final;
  void CreateTextWindow(WindowId window_id, text::Selection* selection) final;
  void DestroyWindow(WindowId window_id) final;
  void DidStartScriptHost(domapi::ScriptHostState state) final;
  void FocusWindow(WindowId window_id) final;
  void GetFileNameForLoad(WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForLoadResolver& callback) final;
  base::string16 GetMetrics(const base::string16& name) final;
  void GetFileNameForSave(WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForSaveResolver& resolver) final;
  domapi::SwitchValue GetSwitch(const base::string16& name) final;
  std::vector<base::string16> GetSwitchNames() final;
  std::vector<int> GetTableRowStates(
      WindowId window_id,
      const std::vector<base::string16>& keys) final;
  void HideWindow(WindowId window_id) final;
  domapi::FloatRect HitTestTextPosition(WindowId window_id,
                                        text::Posn position);
  void MakeSelectionVisible(WindowId window_id) final;
  text::Posn MapPointToPosition(domapi::EventTargetId event_target_id,
                                float x,
                                float y) final;
  void MessageBox(WindowId window_id,
                  const base::string16& message,
                  const base::string16& title,
                  int flags,
                  const MessageBoxResolver& resolver) final;
  void Reconvert(WindowId window_id, const base::string16& text) final;
  void RealizeWindow(WindowId window_id) final;
  void ReleaseCapture(domapi::EventTargetId event_target_id) final;
  void ScrollTextWindow(WindowId window_id, int direction) final;
  void SetCapture(domapi::EventTargetId event_target_id) final;
  void SetStatusBar(WindowId window_id,
                    const std::vector<base::string16>& texts) final;
  void SetSwitch(const base::string16& name,
                 const domapi::SwitchValue& new_value) final;
  void SetTabData(WindowId window_id, const domapi::TabData& tab_data) final;
  void SetTextWindowZoom(WindowId window_id, float zoom) final;
  void ShowWindow(WindowId window_id) final;
  void SplitHorizontally(WindowId left_window_id,
                         WindowId new_right_window_id) final;
  void SplitVertically(WindowId above_window_id,
                       WindowId new_below_window_id) final;
  void UpdateWindow(WindowId window_id) final;

  // domapi::ViewEventHandler
  void DidBeginFrame(const base::Time& deadline);
  void DidChangeWindowBounds(WindowId window_id,
                             int left,
                             int top,
                             int right,
                             int bottom) final;
  void DidChangeWindowVisibility(WindowId window_id,
                                 domapi::Visibility visibility) final;
  void DidDestroyWidget(WindowId window_id) final;
  void DidDropWidget(WindowId source_id, WindowId target_id) final;
  void DidRealizeWidget(WindowId window_id) final;
  void DispatchFocusEvent(const domapi::FocusEvent& event) final;
  void DispatchKeyboardEvent(const domapi::KeyboardEvent& event) final;
  void DispatchMouseEvent(const domapi::MouseEvent& event) final;
  void DispatchTextCompositionEvent(
      const domapi::TextCompositionEvent& event) final;
  void DispatchWheelEvent(const domapi::WheelEvent& event) final;
  void OpenFile(WindowId window_id, const base::string16& file_name) final;
  void ProcessCommandLine(base::string16 working_directory,
                          const std::vector<base::string16>& args) final;
  void QueryClose(WindowId window_id) final;
  void RunCallback(base::Closure callback) final;
  void WillDestroyViewHost() final;

  domapi::IoDelegate* io_delegate_;
  base::MessageLoop* io_message_loop_;
  const std::unique_ptr<base::Thread> thread_;
  ViewDelegate* const view_delegate_;
  base::MessageLoop* view_message_loop_;
  std::unique_ptr<base::WaitableEvent> waitable_event_;

  const std::unique_ptr<ScriptHost> script_host_;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#endif  // EVITA_DOM_SCRIPT_THREAD_H_
