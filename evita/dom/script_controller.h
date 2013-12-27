// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_controller_h)
#define INCLUDE_evita_dom_script_controller_h

#include <memory>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100 4625)
#include "base/bind.h"
#include "base/callback.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "base/threading/thread_checker.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/isolate_holder.h"
#include "evita/vi_Buffer.h"
BEGIN_V8_INCLUDE
#include "gin/public/context_holder.h"
END_V8_INCLUDE

namespace base {
template<typename T> class Callback;
}

namespace dom {

struct EvaluateResult {
  base::string16 value;
  base::string16 script_resource_name;
  base::string16 exception;
  base::string16 source_line;
  int line_number;
  int start_column;
  int end_column;
  std::vector<base::string16> stack_trace;

  EvaluateResult() : line_number(0), start_column(0), end_column(0) {
  }
};

class ScriptController : public common::Singleton<ScriptController> {
  friend class common::Singleton<ScriptController>;

  public: class User : public common::Singleton<User> {
    friend class common::Singleton<User>;

    private: base::ThreadChecker thread_checker_;

    private: User() = default;
    public: ~User() = default;

    public: void Evaluate(base::string16 script_text,
                          base::Callback<void(EvaluateResult)> callback);

    DISALLOW_COPY_AND_ASSIGN(User);
  };
  friend class User;

  private: v8_glue::IsolateHolder isolate_holder_;
  private: gin::ContextHolder context_holder_;
  private: base::ThreadChecker thread_checker_;

  private: ScriptController();
  public: ~ScriptController();

  // TODO(yosi) |ScriptController::Evaluate| should be private.
  public: void Evaluate(base::string16 script_text,
                         base::Callback<void(EvaluateResult)> callback);

  public: static void Start();

  DISALLOW_COPY_AND_ASSIGN(ScriptController);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_controller_h)
