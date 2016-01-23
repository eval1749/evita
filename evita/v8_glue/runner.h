// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_V8_GLUE_RUNNER_H_
#define EVITA_V8_GLUE_RUNNER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/memory/weak_ptr.h"
#include "evita/v8_glue/context_holder.h"
#include "gin/runner.h"

namespace v8_glue {

class RunnerDelegate;

class Runner : public gin::Runner {
 private:
  class CurrentRunnerScope final {
   public:
    explicit CurrentRunnerScope(Runner* runner);
    ~CurrentRunnerScope();

   private:
    v8::Isolate* const isolate_;

    DISALLOW_COPY_AND_ASSIGN(CurrentRunnerScope);
  };

 public:
  typedef std::vector<v8::Local<v8::Value>> Args;

  class EscapableHandleScope final {
   public:
    explicit EscapableHandleScope(Runner* runner);
    ~EscapableHandleScope();

    template <typename T>
    v8::Local<T> Escape(v8::Local<T> value) {
      return handle_scope_.Escape<T>(value);
    }

   private:
    v8::Locker isolate_locker_;
    v8::Isolate::Scope isolate_scope_;
    v8::EscapableHandleScope handle_scope_;
    v8::Context::Scope context_scope_;
    CurrentRunnerScope current_runner_scope_;
    Runner* const runner_;

    DISALLOW_COPY_AND_ASSIGN(EscapableHandleScope);
  };

  class Scope final {
   public:
    explicit Scope(Runner* runner);
    ~Scope();

   private:
    v8::Locker isolate_locker_;
    v8::Isolate::Scope isolate_scope_;
    v8::HandleScope handle_scope_;
    v8::Context::Scope context_scope_;
    CurrentRunnerScope current_runner_scope_;
    Runner* const runner_;

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  explicit Runner(v8::Isolate* isolate, RunnerDelegate* delegate);
  ~Runner();

  v8::Local<v8::Context> context() const;
  v8::Isolate* isolate() const;

  static Runner* current_runner(v8::Isolate* isolate);

  v8::Local<v8::Value> CallAsFunction(v8::Local<v8::Value> callee,
                                      v8::Local<v8::Value> receiver,
                                      const Args& args);
  v8::Local<v8::Value> CallAsConstructor(v8::Local<v8::Value> callee,
                                         const Args& args);
  template <typename... Params>
  v8::Local<v8::Value> CallAsFunction(v8::Local<v8::Value> callee,
                                      v8::Local<v8::Value> receiver,
                                      Params... params);
  template <typename... Params>
  v8::Local<v8::Value> CallAsConstructor(v8::Local<v8::Value> callee,
                                         Params... params);
  v8::Local<v8::Value> GetGlobalProperty(const base::StringPiece& name);
  void HandleTryCatch(const v8::TryCatch& try_catch);
  v8::Local<v8::Value> Run(const base::string16& script_text,
                           const base::string16& script_name);
  v8::Local<v8::Value> Run(v8::Local<v8::Script> script);

 private:
  friend class EscapableHandleScope;
  friend class Scope;

  bool CheckCallDepth();

  // Compile |script_text| and returns |v8::Script| handle if succeeded,
  // otherwise returns empty handle and calls |UnhandledException| of
  // |delegate_|.
  v8::Local<v8::Script> Compile(const base::string16& script_text,
                                const base::string16& script_name);

  // gin::Runner
  v8::Local<v8::Value> Call(v8::Local<v8::Function> function,
                            v8::Local<v8::Value> receiver,
                            int argc,
                            v8::Local<v8::Value> argv[]) final;
  gin::ContextHolder* GetContextHolder() final;
  void Run(const std::string& source, const std::string& resource_name) final;

  int call_depth_ = 0;
  std::unique_ptr<gin::ContextHolder> context_holder_;
  RunnerDelegate* const delegate_;
#if defined(_DEBUG)
  int in_scope_ = false;
#endif

  DISALLOW_COPY_AND_ASSIGN(Runner);
};

template <typename... Params>
v8::Local<v8::Value> Runner::CallAsFunction(v8::Local<v8::Value> callee,
                                            v8::Local<v8::Value> receiver,
                                            Params... params) {
  return CallAsFunction(callee, receiver, Args{params...});
}

template <typename... Params>
v8::Local<v8::Value> Runner::CallAsConstructor(v8::Local<v8::Value> callee,
                                               Params... params) {
  return CallAsConstructor(callee, Args{params...});
}

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_RUNNER_H_
