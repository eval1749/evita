// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_thread_h)
#define INCLUDE_evita_dom_script_thread_h

#include <memory>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/location.h"
#pragma warning(push)
#pragma warning(disable: 4625)
#include "base/threading/thread_checker.h"
#pragma warning(pop)
#include "evita/dom/view_delegate.h"
#include "evita/dom/view_event_handler.h"

namespace base {
class MessageLoop;
class Thread;
}

namespace dom {

class EditorWindow;
struct EvaluateResult;
class TextWindow;

class ScriptThread final : public ViewDelegate,
                           public ViewEventHandler {
  private: ViewDelegate* view_delegate_;
  private: base::MessageLoop* host_message_loop_;
  private: std::unique_ptr<base::Thread> thread_;
  private: base::ThreadChecker thread_checker_;
  private: ViewEventHandler* view_event_handler_;

  private: ScriptThread(ViewDelegate* view_delegate,
                        base::MessageLoop* host_message_loop);
  public: ~ScriptThread();

  public: static ScriptThread* instance();

  public: bool CalledOnValidThread() const;
  private: bool CalledOnScriptThread() const;

// TODO(yosi) We will remove Evaluate once V8Console in JS.
  public: void Evaluate(const base::string16& script_text,
                        base::Callback<void(EvaluateResult)> callback);
  private: void EvaluateImpl(base::string16 script_text,
                             base::Callback<void(EvaluateResult)> callback);

  // TODO(yosi) we will make ScriptThread::PostTask() private.
  public: void PostTask(const tracked_objects::Location& from_here,
                        const base::Closure& task);

  public: static void Start(ViewDelegate* view_delegate,
                            base::MessageLoop* host_message_loop);

  // ViewDelegate
  private: virtual void CreateEditorWindow(
      const EditorWindow* window) override;
  private: virtual void CreateTextWindow(const TextWindow* window) override;
  private: virtual void AddWindow(WidgetId parent_id,
                                  WidgetId child_id) override;
  private: virtual void DestroyWindow(WidgetId widget_id) override;
  private: virtual void RealizeWindow(WidgetId widget_id) override;
  private: virtual void RegisterViewEventHandler(
      ViewEventHandler* event_handler) override;

  // ViewEventHandler
  private: virtual void DidDestroyWidget(WidgetId widget_id) override;
  private: virtual void DidRealizeWidget(WidgetId widget_id) override;
  private: virtual void WillDestroyHost() override;

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

// TODO(yosi) We will remove this once we use ViewDelegate and ViewEventHandler.
#define ASSERT_CALLED_ON_SCRIPT_THREAD() \
  DCHECK(dom::ScriptThread::instance()->CalledOnValidThread())

#endif //!defined(INCLUDE_evita_dom_script_thread_h)
