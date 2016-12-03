// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_ENGINE_native_script_module_H_
#define EVITA_DOM_ENGINE_native_script_module_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/ginx/scriptable.h"

namespace dom {

namespace bindings {
class NativeScriptModuleClass;
}

class ExceptionState;
class ScriptHost;

class NativeScriptModule final : public ginx::Scriptable<NativeScriptModule> {
  DECLARE_SCRIPTABLE_OBJECT(NativeScriptModule)

 public:
  ~NativeScriptModule() final;

  v8::Local<v8::Module> ToV8(v8::Isolate* isolate) const;

 private:
  friend class bindings::NativeScriptModuleClass;

  enum class State;

  NativeScriptModule(v8::Isolate* isolate,
                     v8::Local<v8::Module> module,
                     const base::string16& specifier);

  // NativeScriptModule.idl
  int id(ScriptHost* script_host) const;
  std::vector<base::string16> requests(ScriptHost* script_host) const;
  const base::string16& specifier() const { return specifier_; }

  static NativeScriptModule* Compile(ScriptHost* script_host,
                                     const base::string16& specifier,
                                     const base::string16& script_text,
                                     ExceptionState* exception_state);

  void Evaluate(ScriptHost* script_host, ExceptionState* exception_state);

  void Instantiate(ScriptHost* script_host,
                   v8::Local<v8::Function> callback,
                   ExceptionState* exception_state);

  v8::Global<v8::Module> module_;
  const base::string16 specifier_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(NativeScriptModule);
};

}  // namespace dom

#endif  // EVITA_DOM_ENGINE_native_script_module_H_
