// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_script_thread_h)
#define INCLUDE_evita_dom_script_thread_h

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
class WaitableEvent;
}

namespace dom {

class ScriptThread final : public domapi::IoDelegate,
                           public ViewDelegate,
                           public domapi::ViewEventHandler {
  private: domapi::IoDelegate* io_delegate_;
  private: base::MessageLoop* io_message_loop_;
  private: ViewDelegate* view_delegate_;
  private: domapi::ViewEventHandler* view_event_handler_;
  private: base::MessageLoop* view_message_loop_;
  private: std::unique_ptr<base::WaitableEvent> waitable_event_;

  private: ScriptThread(base::MessageLoop* host_message_loop,
                        ViewDelegate* view_delegate,
                        base::MessageLoop* io_message_loop,
                        domapi::IoDelegate* io_delegate);
  public: ~ScriptThread() final;

  public: static void Start(base::MessageLoop* host_message_loop,
                            ViewDelegate* view_delegate,
                            base::MessageLoop* io_message_loop,
                            domapi::IoDelegate* io_delegate);

  // domapi::IoDelegate
  private: void CheckSpelling(const base::string16& word_to_check,
        const CheckSpellingResolver& deferred) final;
  private: void CloseFile(domapi::IoContextId context_id,
      const domapi::FileIoDeferred& deferred) final;
  private: void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& deferred) final;
  private: void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) final;
  private: void MoveFile(
      const base::string16& src_path, const base::string16& dst_path,
      const domapi::MoveFileOptions& options,
      const domapi::IoResolver& resolver) final;
  private: void OpenFile(const base::string16& file_name,
      const base::string16& mode,
      const domapi::OpenFileDeferred& deferred) final;
  private: void OpenProcess(const base::string16& command_line,
      const domapi::OpenProcessDeferred& deferred) final;
  private: void QueryFileStatus(const base::string16& file_name,
      const domapi::QueryFileStatusDeferred& deferred) final;
  private: void ReadFile(domapi::IoContextId context_id,
      void* buffer, size_t num_read,
      const domapi::FileIoDeferred& deferred) final;
  private: void RemoveFile(
      const base::string16& file_name,
      const domapi::IoResolver& resolver) final;
  private: void WriteFile(domapi::IoContextId context_id,
      void* buffer, size_t num_write,
      const domapi::FileIoDeferred& deferred) final;

  // ViewDelegate
  private: void AddWindow(WindowId parent_id, WindowId child_id) final;
  public: void ChangeParentWindow(WindowId window_id,
                                  WindowId new_parent_window_id) final;
  private: text::Posn ComputeOnTextWindow(WindowId window_id,
                                          const TextWindowCompute& data) final;
  private: void CreateEditorWindow(const EditorWindow* window) final;
  private: void CreateFormWindow(WindowId window_id, Form* form,
                                 const domapi::PopupWindowInit& init) final;
  private: void CreateTableWindow(WindowId window_id, Document* document) final;
  private: void CreateTextWindow(WindowId window_id,
                                 text::Selection* selection) final;
  private: void DestroyWindow(WindowId window_id) final;
  private: void DidStartScriptHost(domapi::ScriptHostState state) final;
  private: void FocusWindow(WindowId window_id) final;
  private: void GetFileNameForLoad(
      WindowId window_id, const base::string16& dir_path,
      const GetFileNameForLoadResolver& callback) final;
  private: base::string16 GetMetrics(const base::string16& name) final;
  private: void GetFileNameForSave(WindowId window_id,
                                   const base::string16& dir_path,
      const GetFileNameForSaveResolver& resolver) final;
  private: domapi::SwitchValue GetSwitch(const base::string16& name) final;
  private: std::vector<base::string16> GetSwitchNames() final;
  private: std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) final;
  private: void HideWindow(WindowId window_id) final;
  private: domapi::FloatRect HitTestTextPosition(
      WindowId window_id, text::Posn position);
  private: void MakeSelectionVisible(WindowId window_id) final;
  private: text::Posn MapPointToPosition(domapi::EventTargetId event_target_id,
                                         float x, float y) final;
  private: void MessageBox(WindowId window_id,
                           const base::string16& message,
                           const base::string16& title, int flags,
                           const MessageBoxResolver& resolver) final;
  private: void Reconvert(WindowId window_id, const base::string16& text) final;
  private: void RealizeWindow(WindowId window_id) final;
  private: void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) final;
  private: void ReleaseCapture(domapi::EventTargetId event_target_id) final;
  private: void ScrollTextWindow(WindowId window_id, int direction) final;
  private: void SetCapture(domapi::EventTargetId event_target_id) final;
  private: void SetStatusBar(WindowId window_id,
      const std::vector<base::string16>& texts) final;
  private: void SetSwitch(const base::string16& name,
                          const domapi::SwitchValue& new_value) final;
  private: void SetTabData(WindowId window_id,
                           const domapi::TabData& tab_data) final;
  private: void SetTextWindowZoom(WindowId window_id,
                                          float zoom) final;
  private: void ShowWindow(WindowId window_id) final;
  private: void SplitHorizontally(WindowId left_window_id,
      WindowId new_right_window_id) final;
  private: void SplitVertically(WindowId above_window_id,
      WindowId new_below_window_id) final;
  private: void UpdateWindow(WindowId window_id) final;

  // domapi::ViewEventHandler
  private: void DidChangeWindowBounds(
      WindowId window_id, int left, int top, int right, int bottom) final;
  private: void DidChangeWindowVisibility(
      WindowId window_id, domapi::Visibility visibility) final;
  private: void DidDestroyWidget(WindowId window_id) final;
  private: void DidDropWidget(WindowId source_id, WindowId target_id) final;
  private: void DidRealizeWidget(WindowId window_id) final;
  private: void DidStartViewHost() final;
  private: void DispatchFocusEvent(const domapi::FocusEvent& event) final;
  private: void DispatchKeyboardEvent(const domapi::KeyboardEvent& event) final;
  private: void DispatchMouseEvent(const domapi::MouseEvent& event) final;
  private: void DispatchTextCompositionEvent(
      const domapi::TextCompositionEvent& event) final;
  private: void DispatchWheelEvent(const domapi::WheelEvent& event) final;
  private: void OpenFile(WindowId window_id,
                         const base::string16& file_name) final;
  private: void QueryClose(WindowId window_id) final;
  private: void RunCallback(base::Closure callback) final;
  private: void WillDestroyHost() final;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_thread_h)
