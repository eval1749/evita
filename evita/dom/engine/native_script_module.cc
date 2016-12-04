// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <unordered_map>

#include "evita/dom/engine/native_script_module.h"

#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/script_host.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/runner.h"

namespace dom {

namespace {

// Map v8::Module to NativeScriptModule.
class ModuleMap final {
 public:
  ~ModuleMap() = default;

  NativeScriptModule* Get(v8::Local<v8::Module> module) const;
  void Set(v8::Local<v8::Module> module, NativeScriptModule* script_module);

  static ModuleMap* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ModuleMap>;

  ModuleMap() = default;

  std::unordered_map<int, NativeScriptModule*> map_;

  DISALLOW_COPY_AND_ASSIGN(ModuleMap);
};

NativeScriptModule* ModuleMap::Get(v8::Local<v8::Module> module) const {
  const auto& it = map_.find(module->GetIdentityHash());
  DCHECK(it != map_.end());
  return it->second;
}

ModuleMap* ModuleMap::GetInstance() {
  return base::Singleton<ModuleMap>::get();
}

void ModuleMap::Set(v8::Local<v8::Module> module,
                    NativeScriptModule* script_module) {
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
  NativeScriptModule* script_module = nullptr;
  if (!gin::ConvertFromV8(isolate, result, &script_module))
    return v8::MaybeLocal<v8::Module>();
  return v8::MaybeLocal<v8::Module>(script_module->ToV8(isolate));
}

ModuleResolver* ModuleResolver::GetInstance() {
  return base::Singleton<ModuleResolver>::get();
}

bool IsInstanceOf(v8::Local<v8::Context> context,
                  v8::Local<v8::Object> object,
                  v8::Local<v8::Object> clazz) {
  DCHECK(clazz->IsConstructor());
  auto* const isolate = context->GetIsolate();
  auto constructorKey = gin::StringToV8(isolate, "constructor");
  for (v8::Local<v8::Value> runner = object; runner->IsObject();
       runner = runner->ToObject()->GetPrototype()) {
    auto constructor = runner->ToObject(context)
                           .ToLocalChecked()
                           ->Get(context, constructorKey)
                           .FromMaybe(v8::Local<v8::Value>());
    if (constructor == clazz)
      return true;
  }
  return false;
}

// Populate Mozilla compiatible |fileName|, |lineNumber| and |columnNumber|
// properties to |Error| object if possible.
void ProcessMessage(v8::Local<v8::Context> context,
                    const v8::TryCatch& try_catch) {
  auto const message = try_catch.Message();
  if (message.IsEmpty())
    return;
  auto const js_error = try_catch.Exception()->ToObject(context).FromMaybe(
      v8::Local<v8::Object>());
  if (js_error.IsEmpty())
    return;
  auto* const isolate = context->GetIsolate();
  auto const error_class = context->Global()
                               ->Get(context, gin::StringToV8(isolate, "Error"))
                               .ToLocalChecked()
                               ->ToObject(context)
                               .FromMaybe(v8::Local<v8::Object>());
  if (!IsInstanceOf(context, js_error, error_class))
    return;

  auto const fileNameKey = gin::StringToV8(isolate, "fileName");
  if (!js_error->Has(context, fileNameKey).FromMaybe(false))
    js_error->Set(fileNameKey, message->GetScriptResourceName());

  auto const sourceLineKey = gin::StringToV8(isolate, "sourceLine");
  if (!js_error->Has(context, sourceLineKey).FromMaybe(false)) {
    auto sourceLine =
        message->GetSourceLine(context).FromMaybe(v8::Local<v8::String>());
    if (!sourceLine.IsEmpty())
      js_error->Set(gin::StringToV8(isolate, "sourceLine"), sourceLine);
  }

  auto const lineNumberKey = gin::StringToV8(isolate, "lineNumber");
  if (!js_error->Has(context, lineNumberKey).FromMaybe(false)) {
    const int lineNumber = message->GetLineNumber(context).FromMaybe(0);
    js_error->Set(gin::StringToV8(isolate, "lineNumber"),
                  gin::ConvertToV8(isolate, lineNumber));
  }

  auto const columnNumberKey = gin::StringToV8(isolate, "columnNumber");
  if (!js_error->Has(context, columnNumberKey).FromMaybe(false)) {
    const int columnNumber = message->GetStartColumn(context).FromMaybe(0);
    js_error->Set(gin::StringToV8(isolate, "columnNumber"),
                  gin::ConvertToV8(isolate, columnNumber));
  }
}

}  // namespace

enum class NativeScriptModule::State {
  Evaluated,
  Failed,
  Instantiated,
  Created,
  Succeeded,
};

NativeScriptModule::NativeScriptModule(v8::Isolate* isolate,
                                       v8::Local<v8::Module> module,
                                       const base::string16& specifier)
    : module_(isolate, module), specifier_(specifier), state_(State::Created) {}

NativeScriptModule::~NativeScriptModule() = default;

int NativeScriptModule::id(ScriptHost* script_host) const {
  auto* const isolate = script_host->isolate();
  return ToV8(isolate)->GetIdentityHash();
}

std::vector<base::string16> NativeScriptModule::requests(
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

NativeScriptModule* NativeScriptModule::Compile(
    ScriptHost* script_host,
    const base::string16& specifier,
    const base::string16& script_text,
    ExceptionState* exception_state) {
  auto* const isolate = script_host->isolate();
  auto* const runner = script_host->runner();
  ginx::Runner::Scope runner_scope(runner);
  v8::TryCatch try_catch(isolate);
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, specifier));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  auto module = v8::ScriptCompiler::CompileModule(isolate, &source)
                    .FromMaybe(v8::Local<v8::Module>());
  if (module.IsEmpty()) {
    ProcessMessage(runner->context(), try_catch);
    try_catch.ReThrow();
    exception_state->set_is_thrown();
    return nullptr;
  }
  auto* const script_module =
      new NativeScriptModule(isolate, module, specifier);
  ModuleMap::GetInstance()->Set(module, script_module);
  return script_module;
}

v8::Local<v8::Value> NativeScriptModule::Evaluate(
    ScriptHost* script_host,
    ExceptionState* exception_state) {
  if (state_ != State::Instantiated) {
    exception_state->ThrowError(base::StringPrintf(
        "Can not evaluate Module '%ls' state=%d.", specifier_.c_str(), state_));
    return v8::Local<v8::Value>();
  }
  auto* const isolate = script_host->isolate();
  auto context = script_host->runner()->context();
  auto module = ToV8(isolate);
  auto result = module->Evaluate(context);
  if (result.IsEmpty()) {
    state_ = State::Failed;
    exception_state->set_is_thrown();
    return v8::Local<v8::Value>();
  }
  state_ = State::Succeeded;
  return result.ToLocalChecked();
}

void NativeScriptModule::Instantiate(ScriptHost* script_host,
                                     v8::Local<v8::Function> callback,
                                     ExceptionState* exception_state) {
  if (state_ != State::Created) {
    exception_state->ThrowError(
        base::StringPrintf("Can not Instantiate moudle '%ls'state=%d.",
                           specifier_.c_str(), state_));
    return;
  }
  auto* const isolate = script_host->isolate();
  auto context = script_host->runner()->context();
  auto module = ToV8(isolate);
  ModuleResolver::Scope scope(callback);
  if (!module->Instantiate(context, ModuleResolver::Callback)) {
    state_ = State::Failed;
    exception_state->set_is_thrown();
    return;
  }
  state_ = State::Instantiated;
}

v8::Local<v8::Module> NativeScriptModule::ToV8(v8::Isolate* isolate) const {
  return module_.Get(isolate);
}

}  // namespace dom
