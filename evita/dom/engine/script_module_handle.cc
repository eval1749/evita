// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <unordered_map>

#include "evita/dom/engine/script_module_handle.h"

#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/script_host.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/runner.h"

namespace dom {

namespace {

// Map v8::Module to ScriptModuleHandle.
class ModuleMap final {
 public:
  ~ModuleMap() = default;

  ScriptModuleHandle* Get(v8::Local<v8::Module> module) const;
  void Set(v8::Local<v8::Module> module, ScriptModuleHandle* script_module);

  static ModuleMap* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ModuleMap>;

  ModuleMap() = default;

  std::unordered_map<int, ScriptModuleHandle*> map_;

  DISALLOW_COPY_AND_ASSIGN(ModuleMap);
};

ScriptModuleHandle* ModuleMap::Get(v8::Local<v8::Module> module) const {
  const auto& it = map_.find(module->GetIdentityHash());
  DCHECK(it != map_.end());
  return it->second;
}

ModuleMap* ModuleMap::GetInstance() {
  return base::Singleton<ModuleMap>::get();
}

void ModuleMap::Set(v8::Local<v8::Module> module, ScriptModuleHandle* script_module) {
  const auto& result = map_.emplace(module->GetIdentityHash(), script_module);
  DCHECK(result.second);
}

class ModuleResolver final {
 public:
  class Scope final {
   public:
    explicit Scope(v8::Local<v8::Function>);
    ~Scope();

   private:
    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  ~ModuleResolver() = default;

  static v8::MaybeLocal<v8::Module> Callback(v8::Local<v8::Context> context,
                                             v8::Local<v8::String> specifier,
                                             v8::Local<v8::Module> referrer);

  static ModuleResolver* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ModuleResolver>;

  ModuleResolver() = default;

  std::stack<v8::Local<v8::Function>> stack_;

  DISALLOW_COPY_AND_ASSIGN(ModuleResolver);
};

ModuleResolver::Scope::Scope(v8::Local<v8::Function> callback) {
  ModuleResolver::GetInstance()->stack_.push(callback);
}

ModuleResolver::Scope::~Scope() {
  ModuleResolver::GetInstance()->stack_.pop();
}

v8::MaybeLocal<v8::Module> ModuleResolver::Callback(
    v8::Local<v8::Context> context,
    v8::Local<v8::String> specifier,
    v8::Local<v8::Module> referrer) {
  auto* const runner = ginx::Runner::From(context);
  auto* const isolate = runner->isolate();
  auto callback = GetInstance()->stack_.top();
  auto script_referrer = ModuleMap::GetInstance()->Get(referrer);
  auto result =
      runner->CallAsFunction(callback, v8::Undefined(isolate), specifier,
                             gin::ConvertToV8(isolate, script_referrer));
  if (result.IsEmpty())
    return v8::MaybeLocal<v8::Module>();
  ScriptModuleHandle* script_module = nullptr;
  if (!gin::ConvertFromV8(isolate, result, &script_module))
    return v8::MaybeLocal<v8::Module>();
  return v8::MaybeLocal<v8::Module>(script_module->ToV8(isolate));
}

ModuleResolver* ModuleResolver::GetInstance() {
  return base::Singleton<ModuleResolver>::get();
}

}  // namespace

enum class ScriptModuleHandle::State {
  Evaluated,
  Instantiated,
  Created,
  Ready,
};

ScriptModuleHandle::ScriptModuleHandle(v8::Isolate* isolate,
                           v8::Local<v8::Module> module,
                           const base::string16& specifier)
    : module_(isolate, module), specifier_(specifier), state_(State::Created) {}

ScriptModuleHandle::~ScriptModuleHandle() = default;

int ScriptModuleHandle::id(ScriptHost* script_host) const {
  auto* const isolate = script_host->isolate();
  return ToV8(isolate)->GetIdentityHash();
}

std::vector<base::string16> ScriptModuleHandle::requests(
    ScriptHost* script_host) const {
  auto* const isolate = script_host->isolate();
  auto module = ToV8(isolate);
  const auto request_count = module->GetModuleRequestsLength();
  std::vector<base::string16> requests;
  requests.reserve(request_count);
  for (auto index = 0; index < request_count; ++index) {
    base::string16 request;
    CHECK(
        gin::ConvertFromV8(isolate, module->GetModuleRequest(index), &request));
    requests.push_back(request);
  }
  return requests;
}

ScriptModuleHandle* ScriptModuleHandle::Compile(ScriptHost* script_host,
                                    const base::string16& specifier,
                                    const base::string16& script_text,
                                    ExceptionState* exception_state) {
  auto* const isolate = script_host->isolate();
  auto* const runner = script_host->runner();
  ginx::Runner::Scope runner_scope(runner);
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, specifier));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  auto module = v8::ScriptCompiler::CompileModule(isolate, &source)
                    .FromMaybe(v8::Local<v8::Module>());
  if (module.IsEmpty()) {
    exception_state->set_is_thrown();
    return nullptr;
  }
  auto* const script_module = new ScriptModuleHandle(isolate, module, specifier);
  ModuleMap::GetInstance()->Set(module, script_module);
  return script_module;
}

void ScriptModuleHandle::Evaluate(ScriptHost* script_host,
                            ExceptionState* exception_state) {
  if (state_ != State::Ready) {
    exception_state->ThrowError(
        base::StringPrintf("Module '%ls' isn't ready.", specifier_.c_str()));
    return;
  }
  auto* const isolate = script_host->isolate();
  auto context = script_host->runner()->context();
  auto module = ToV8(isolate);
  state_ = State::Evaluated;
  if (!module->Evaluate(context).IsEmpty())
    return exception_state->set_is_thrown();
}

bool ScriptModuleHandle::Instantiate(ScriptHost* script_host,
                               v8::Local<v8::Function> callback,
                               ExceptionState* exception_state) {
  if (state_ != State::Created) {
    exception_state->ThrowError(base::StringPrintf(
        "Module '%ls' is already instantiated.", specifier_.c_str()));
    return false;
  }
  auto* const isolate = script_host->isolate();
  auto context = script_host->runner()->context();
  auto module = ToV8(isolate);
  ModuleResolver::Scope scope(callback);
  state_ = State::Instantiated;
  if (!module->Instantiate(context, ModuleResolver::Callback)) {
    exception_state->set_is_thrown();
    return false;
  }
  state_ = State::Ready;
  return true;
}

v8::Local<v8::Module> ScriptModuleHandle::ToV8(v8::Isolate* isolate) const {
  return module_.Get(isolate);
}

}  // namespace dom
