// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_ENGINE_SCRIPT_MODULE_HANDLE_H_
#define EVITA_DOM_ENGINE_SCRIPT_MODULE_HANDLE_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/ginx/scriptable.h"

namespace dom {

namespace bindings {
class ScriptModuleHandleClass;
}

class ExceptionState;
class ScriptHost;

class ScriptModuleHandle final : public ginx::Scriptable<ScriptModuleHandle> {
  DECLARE_SCRIPTABLE_OBJECT(ScriptModuleHandle)

 public:
  ~ScriptModuleHandle() final;

  v8::Local<v8::Module> ToV8(v8::Isolate* isolate) const;

 private:
  friend class bindings::ScriptModuleHandleClass;

  enum class State;

  ScriptModuleHandle(v8::Isolate* isolate,
                     v8::Local<v8::Module> module,
                     const base::string16& specifier);

  // ScriptModuleHandle.idl
  int id(ScriptHost* script_host) const;
  std::vector<base::string16> requests(ScriptHost* script_host) const;
  const base::string16& specifier() const { return specifier_; }

  static ScriptModuleHandle* Compile(ScriptHost* script_host,
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

  DISALLOW_COPY_AND_ASSIGN(ScriptModuleHandle);
};

}  // namespace dom

#endif  // EVITA_DOM_ENGINE_SCRIPT_MODULE_HANDLE_H_
