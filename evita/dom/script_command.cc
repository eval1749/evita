// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_command.h"

#include "base/bind.h"
#include "base/logging.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"

namespace dom {

ScriptCommand::ScriptCommand(v8::Handle<v8::Object> command)
    : command_(v8::Isolate::GetCurrent(), command) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
}

void ScriptCommand::Execute(const ::Command::Context* context) {
  DCHECK(context);
  ASSERT_CALLED_ON_UI_THREAD();
  // TODO(yosi) We should manage life time of Command::Context.
  ScriptThread::instance()->PostTask(FROM_HERE,
      base::Bind(&ScriptCommand::RunCommand, this,
                 base::Unretained(context)));
}

void ScriptCommand::RunCommand(const ::Command::Context* context) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  DCHECK(context);
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  DOM_AUTO_LOCK_SCOPE();
  auto command = command_.NewLocal(isolate);
  if (command->IsCallable()) {
    command->CallAsFunction(v8::Null(isolate), 0, nullptr);
    return;
  }
  auto value = command->Get(gin::StringToSymbol(isolate, "value"));
  if (value.IsEmpty() || !value->IsObject()) {
    DVLOG(0) << "Command object doesn't have 'value' property.";
    return;
  }
  auto function = value->ToObject();
  if (!function->IsCallable()) {
    DVLOG(0) << "Command object doesn't have callable object.";
    return;
  }
  function->CallAsFunction(v8::Null(isolate), 0, nullptr);
}

}  // namespace dom
