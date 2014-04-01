// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
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
  public: virtual ~ScriptThread();

  public: static void Start(base::MessageLoop* host_message_loop,
                            ViewDelegate* view_delegate,
                            base::MessageLoop* io_message_loop,
                            domapi::IoDelegate* io_delegate);

  // domapi::IoDelegate
  private: virtual void CloseFile(domapi::IoContextId context_id,
      const domapi::FileIoDeferred& deferred) override;
  private: virtual void OpenFile(const base::string16& filename,
      const base::string16& mode,
      const domapi::OpenFileDeferred& deferred) override;
  private: virtual void OpenProcess(
      const base::string16& command_line,
      const domapi::OpenProcessDeferred& deferred) override;
  private: virtual void QueryFileStatus(const base::string16& filename,
      const domapi::QueryFileStatusDeferred& deferred) override;
  private: virtual void ReadFile(domapi::IoContextId context_id,
      void* buffer, size_t num_read,
      const domapi::FileIoDeferred& deferred) override;
  private: virtual void WriteFile(domapi::IoContextId context_id,
      void* buffer, size_t num_write,
      const domapi::FileIoDeferred& deferred) override;

  // ViewDelegate
  private: virtual void AddWindow(WindowId parent_id,
                                  WindowId child_id) override;
  public: virtual void ChangeParentWindow(WindowId window_id,
    WindowId new_parent_window_id) override;
  private: virtual void CheckSpelling(const base::string16& word_to_check,
        const CheckSpellingDeferred& deferred) override;
  private: virtual text::Posn ComputeOnTextWindow(
      WindowId window_id, const TextWindowCompute& data) override;
  private: virtual void CreateEditorWindow(
      const EditorWindow* window) override;
  private: virtual void CreateFormWindow(WindowId window_id,
                                         const Form* form) override;
  private: virtual void CreateTableWindow(
      WindowId window_id, Document* document) override;
  private: virtual void CreateTextWindow(const TextWindow* window) override;
  private: virtual void DestroyWindow(WindowId window_id) override;
  private: virtual void DidStartScriptHost(
      domapi::ScriptHostState state) override;
  private: virtual void DidHandleViewIdelEvent(int hint) override;
  private: virtual void FocusWindow(WindowId window_id) override;
  private: virtual void GetFilenameForLoad(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) override;
  private: virtual base::string16 GetMetrics(
      const base::string16& name) override;
  private: virtual void GetFilenameForSave(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) override;
  private: virtual void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsDeferred& deferred) override;
  private: virtual std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) override;
  private: virtual void HideWindow(WindowId window_id) override;
  private: virtual void LoadFile(Document* document,
                                 const base::string16& filename,
                                 LoadFileCallback callback) override;
  private: virtual void MakeSelectionVisible(WindowId window_id) override;
  private: virtual text::Posn MapPointToPosition(WindowId window_id,
                                                 float x, float y) override;
  private: virtual domapi::FloatPoint MapPositionToPoint(
      WindowId window_id, text::Posn position);
  private: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) override;
  private: virtual void Reconvert(WindowId window_id, text::Posn start,
                                  text::Posn end) override;
  private: virtual void RealizeWindow(WindowId window_id) override;
  private: virtual void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) override;
  private: virtual void ReleaseCapture(WindowId window_id) override;
  private: virtual void SaveFile(Document* document,
                                 const base::string16& filename,
                                 const SaveFileCallback& callback) override;
  private: virtual void ScrollTextWindow(WindowId window_id,
                                         int direction) override;
  private: virtual void SetCapture(WindowId window_id) override;
  private: virtual void SetStatusBar(WindowId window_id,
      const std::vector<base::string16>& texts) override;
  private: virtual void SetTabData(WindowId window_id,
                                   const domapi::TabData& tab_data) override;
  private: virtual void ShowWindow(WindowId window_id) override;
  private: virtual void SplitHorizontally(WindowId left_window_id,
      WindowId new_right_window_id) override;
  private: virtual void SplitVertically(WindowId above_window_id,
      WindowId new_below_window_id) override;

  // domapi::ViewEventHandler
  private: virtual void AppendTextToBuffer(text::Buffer* buffer,
                                           const base::string16& text) override;
  private: virtual void DidDestroyWidget(WindowId window_id) override;
  private: virtual void DidDropWidget(WindowId source_id,
                                      WindowId target_id) override;
  private: virtual void DidRealizeWidget(WindowId window_id) override;
  private: virtual void DidResizeWidget(WindowId window_id, int left, int top,
                                        int right, int bottom) override;
  private: virtual void DidStartViewHost() override;
  private: virtual void DispatchFocusEvent(
      const domapi::FocusEvent& event) override;
  private: virtual void DispatchKeyboardEvent(
      const domapi::KeyboardEvent& event) override;
  private: virtual void DispatchMouseEvent(
      const domapi::MouseEvent& event) override;
  private: void DispatchViewIdleEvent(int hint) override;
  private: virtual void DispatchWheelEvent(
      const domapi::WheelEvent& event) override;
  private: virtual void OpenFile(WindowId window_id,
                                 const base::string16& filename) override;
  private: virtual void QueryClose(WindowId window_id) override;
  private: virtual void RunCallback(base::Closure callback) override;
  private: virtual void WillDestroyHost() override;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_thread_h)
