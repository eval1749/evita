// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_controller_h)
#define INCLUDE_evita_dom_script_controller_h

#include <string>
#include <memory>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100 4625)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/dom/view_event_handler.h"
#include "evita/v8_glue/context_holder.h"
#include "evita/v8_glue/isolate_holder.h"
#include "evita/v8_glue/v8.h"

namespace base {
template<typename T> class Callback;
}

namespace dom {

class EventHandler;
class ViewDelegate;

//////////////////////////////////////////////////////////////////////
//
// EvaluateResult
//
// TODO(yosi) We will remove EvaluateResult once V8Console in JS.
struct EvaluateResult {
  base::string16 value;
  base::string16 script_resource_name;
  base::string16 exception;
  base::string16 source_line;
  int line_number;
  int start_column;
  int end_column;
  std::vector<base::string16> stack_trace;

  EvaluateResult(const base::string16& script_text);
  EvaluateResult();
};

//////////////////////////////////////////////////////////////////////
//
// ScriptController
//
class ScriptController {
  private: v8_glue::IsolateHolder isolate_holder_;
  private: gin::ContextHolder context_holder_;
  private: std::unique_ptr<EventHandler> event_handler_;
  private: bool testing_;
  private: ViewDelegate* view_delegate_;

  private: ScriptController(ViewDelegate* view_delegate);
  public: ~ScriptController();

  public: ViewDelegate* view_delegate() const;
  public: EventHandler* event_handler() const { return event_handler_.get(); }
  public: static ScriptController* instance();
  public: v8::Isolate* isolate() const;

  public: void DidStartHost();
  public: EvaluateResult Evaluate(const base::string16& script_text);
  public: void LoadJsLibrary();
  public: void LogException(const v8::TryCatch& try_catch);
  public: void OpenFile(WindowId window_id,
                        const base::string16& filename);
  public: void ResetForTesting();
  public: static ScriptController* Start(ViewDelegate* view_delegate);
  public: static ScriptController* StartForTesting(
      ViewDelegate* view_delegate);
  public: void ThrowError(const std::string& message);
  public: void WillDestroyHost();

  DISALLOW_COPY_AND_ASSIGN(ScriptController);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_controller_h)
