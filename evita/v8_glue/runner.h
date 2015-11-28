// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_V8_GLUE_RUNNER_H_
#define EVITA_V8_GLUE_RUNNER_H_

#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/memory/weak_ptr.h"
#include "evita/v8_glue/context_holder.h"

namespace v8_glue {

class RunnerDelegate;

class Runner : public gin::ContextHolder {
 private:
  class CurrentRunnerScope final {
   public:
    explicit CurrentRunnerScope(Runner* runner);
    ~CurrentRunnerScope();

   private:
    v8::Isolate* isolate_;

    DISALLOW_COPY_AND_ASSIGN(CurrentRunnerScope);
  };

 public:
  typedef std::vector<v8::Handle<v8::Value>> Args;

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
    Runner* runner_;

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
    Runner* runner_;

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };

  explicit Runner(v8::Isolate* isolate, RunnerDelegate* delegate);
  ~Runner();

  static Runner* current_runner(v8::Isolate* isolate);
  v8::Handle<v8::Object> global() const;

  v8::Handle<v8::Value> Call(v8::Handle<v8::Value> callee,
                             v8::Handle<v8::Value> receiver,
                             const Args& args);
  v8::Handle<v8::Value> CallAsConstructor(v8::Handle<v8::Value> callee,
                                          const Args& args);
  template <typename... Params>
  v8::Handle<v8::Value> Call(v8::Handle<v8::Value> callee,
                             v8::Handle<v8::Value> receiver,
                             Params... params);
  template <typename... Params>
  v8::Handle<v8::Value> CallAsConstructor(v8::Handle<v8::Value> callee,
                                          Params... params);
  v8::Handle<v8::Value> GetGlobalProperty(const base::StringPiece& name);
  base::WeakPtr<Runner> GetWeakPtr();
  void HandleTryCatch(const v8::TryCatch& try_catch);
  v8::Handle<v8::Value> Run(const base::string16& script_text,
                            const base::string16& script_name);
  v8::Handle<v8::Value> Run(v8::Handle<v8::Script> script);

 private:
  friend class EscapableHandleScope;
  friend class Scope;

  bool CheckCallDepth();

  // Compile |script_text| and returns |v8::Script| handle if succeeded,
  // otherwise returns empty handle and calls |UnhandledException| of
  // |delegate_|.
  v8::Handle<v8::Script> Compile(const base::string16& script_text,
                                 const base::string16& script_name);

  int call_depth_;
  RunnerDelegate* delegate_;
#if defined(_DEBUG)
  int in_scope_;
#endif
  base::WeakPtrFactory<Runner> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Runner);
};

template <typename... Params>
v8::Handle<v8::Value> Runner::Call(v8::Handle<v8::Value> callee,
                                   v8::Handle<v8::Value> receiver,
                                   Params... params) {
  return Call(callee, receiver, Args{params...});
}

template <typename... Params>
v8::Handle<v8::Value> Runner::CallAsConstructor(v8::Handle<v8::Value> callee,
                                                Params... params) {
  return CallAsConstructor(callee, Args{params...});
}

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_RUNNER_H_
