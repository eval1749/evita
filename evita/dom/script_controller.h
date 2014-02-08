// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_controller_h)
#define INCLUDE_evita_dom_script_controller_h

#include <string>
#include <memory>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/dom/view_event_handler.h"
#include "evita/v8_glue/isolate_holder.h"
#include "evita/v8_glue/runner_delegate.h"
#include "evita/v8_glue/scoped_persistent.h"
#include "evita/v8_glue/v8.h"

namespace base {
template<typename T> class Callback;
}

namespace v8_glue {
class Runner;
}

namespace dom {

class EventHandler;
class ViewDelegate;

//////////////////////////////////////////////////////////////////////
//
// ScriptController
//
class ScriptController : public v8_glue::RunnerDelegate {
  private: v8_glue::IsolateHolder isolate_holder_;
  private: std::unique_ptr<EventHandler> event_handler_;
  private: std::unique_ptr<v8_glue::Runner> runner_;
  private: bool started_;
  private: bool testing_;
  private: v8_glue::Runner* testing_runner_;
  private: ViewDelegate* view_delegate_;

  private: ScriptController(ViewDelegate* view_delegate);
  public: ~ScriptController();

  public: EventHandler* event_handler() const { return event_handler_.get(); }
  public: static ScriptController* instance();
  public: v8_glue::Runner* runner() const;
  public: v8::Isolate* isolate() const;
  public: void set_testing_runner(v8_glue::Runner* runner);
  public: ViewDelegate* view_delegate() const;

  public: void DidStartHost();
  public: void OpenFile(WindowId window_id,
                        const base::string16& filename);
  public: void ResetForTesting();
  public: static ScriptController* Start(ViewDelegate* view_delegate);
  public: static ScriptController* StartForTesting(
      ViewDelegate* view_delegate);
  public: void ThrowError(const std::string& message);
  public: void ThrowException(v8::Handle<v8::Value> exception);
  public: void WillDestroyHost();

  // v8_glue::RunnerDelegate
  private: virtual v8::Handle<v8::ObjectTemplate>
      GetGlobalTemplate(v8_glue::Runner* runner) override;
  private: virtual void UnhandledException(v8_glue::Runner* runner,
                                           const v8::TryCatch& try_catch);

  DISALLOW_COPY_AND_ASSIGN(ScriptController);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_controller_h)
