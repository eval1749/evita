// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_command_h)
#define INCLUDE_evita_dom_script_command_h

#include "base/basictypes.h"
#include "evita/cm_CmdProc.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class ScriptCommand : public Command::Command {
  private: v8_glue::ScopedPersistent<v8::Object> command_;

  private: struct Context;

  public: ScriptCommand(v8::Handle<v8::Object> command);
  public: virtual ~ScriptCommand() = default;

  public: virtual void Execute(
      const ::Command::Context* context) override;

  private: void RunCommand(Context* context);

  DISALLOW_COPY_AND_ASSIGN(ScriptCommand);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_command_h)
