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
#include "evita/v8_glue/isolate_holder.h"
#include "evita/v8_glue/v8.h"
BEGIN_V8_INCLUDE
#include "gin/public/context_holder.h"
END_V8_INCLUDE

namespace base {
template<typename T> class Callback;
}

namespace dom {

class ViewDelegate;

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

class ScriptController : public ViewEventHandler {
  private: v8_glue::IsolateHolder isolate_holder_;
  private: gin::ContextHolder context_holder_;
  private: bool testing_;
  private: ViewDelegate* view_delegate_;

  private: ScriptController(ViewDelegate* view_delegate);
  public: ~ScriptController();

  public: ViewDelegate* view_delegate() const;
  public: static ScriptController* instance();

  public: EvaluateResult Evaluate(const base::string16& script_text);
  public: void ResetForTesting();
  public: void PopulateGlobalTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global_template);
  public: static ScriptController* Start(ViewDelegate* view_delegate);
  public: static ScriptController* StartForTesting(
      ViewDelegate* view_delegate);
  public: void ThrowError(const std::string& message);

  // ViewEventHandler
  private: virtual void DidDestroyWidget(WidgetId widget_id) override;
  private: virtual void DidRealizeWidget(WidgetId widget_id) override;
  private: virtual void DidStartHost() override;
  private: virtual void WillDestroyHost() override;

  DISALLOW_COPY_AND_ASSIGN(ScriptController);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_controller_h)
