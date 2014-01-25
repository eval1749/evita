// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_command.h"

// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/logging.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_thread.h"
#include "evita/dom/window.h"
#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"
#include "evita/views/command_window.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// ScriptCommand::Context
//
struct ScriptCommand::Context {
  int arg;
  bool has_arg;
  WindowId window_id;

  Context(const ::Command::Context* context)
      : arg(context->GetArg()), has_arg(context->HasArg()),
        window_id(context->GetWindow()->window_id()) {
  }
};

//////////////////////////////////////////////////////////////////////
//
// ScriptCommand
//
ScriptCommand::ScriptCommand(v8::Handle<v8::Object> command)
    : command_(v8::Isolate::GetCurrent(), command) {
  //ASSERT_CALLED_ON_SCRIPT_THREAD();
}

void ScriptCommand::Execute(const ::Command::Context* context) {
  DCHECK(context);
  ASSERT_CALLED_ON_UI_THREAD();
  auto command_context = new Context(context);
  ScriptThread::instance()->PostTask(FROM_HERE,
      base::Bind(&ScriptCommand::RunCommand, this,
                 base::Unretained(command_context)));
}

void ScriptCommand::RunCommand(Context* context) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  DCHECK(context);
  std::unique_ptr<Context> scoped_context(context);
  auto const active_window = Window::FromWindowId(context->window_id);
  if (!active_window)
    return;

  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  const auto receiver = active_window->GetWrapper(isolate);

  v8::Handle<v8::Value> argv[1];
  auto const argc = context->has_arg ? 1 : 0;
  if (argc)
    argv[0] = v8::Integer::New(isolate, context->arg);

  DOM_AUTO_LOCK_SCOPE();
  auto command = command_.NewLocal(isolate);
  if (command->IsCallable()) {
    command->CallAsFunction(receiver, argc, argv);
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
  function->CallAsFunction(receiver, argc, argv);
}

}  // namespace dom
