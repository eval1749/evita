// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCRIPT_MODULE_H_
#define EVITA_DOM_SCRIPT_MODULE_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/ginx/scriptable.h"

namespace dom {

namespace bindings {
class ScriptModuleClass;
}

class ExceptionState;
class ScriptHost;

class ScriptModule final : public ginx::Scriptable<ScriptModule> {
  DECLARE_SCRIPTABLE_OBJECT(ScriptModule)

 public:
  ~ScriptModule() final;

  v8::Local<v8::Module> ToV8(v8::Isolate* isolate) const;

 private:
  friend class bindings::ScriptModuleClass;

  enum class State;

  ScriptModule(v8::Isolate* isolate,
               v8::Local<v8::Module> module,
               const base::string16& specifier);

  // ScriptModule.idl
  int id(ScriptHost* script_host) const;
  std::vector<base::string16> requests(ScriptHost* script_host) const;
  const base::string16& specifier() const { return specifier_; }

  static ScriptModule* Compile(ScriptHost* script_host,
                               const base::string16& specifier,
                               const base::string16& script_text,
                               ExceptionState* exception_state);

  void Evaluate(ScriptHost* script_host, ExceptionState* exception_state);

  bool Instantiate(ScriptHost* script_host,
                   v8::Local<v8::Function> callback,
                   ExceptionState* exception_state);

  v8::Global<v8::Module> module_;
  const base::string16 specifier_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(ScriptModule);
};

}  // namespace dom

#endif  // EVITA_DOM_SCRIPT_MODULE_H_
