// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_closure_h)
#define INCLUDE_evita_v8_glue_closure_h

#include <vector>
#include <utility>

// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/callback.h"
#pragma warning(pop)
#include "base/memory/weak_ptr.h"
#include "evita/v8_glue/converter.h"

namespace v8_glue {

class Runner;

namespace internal {

class ScriptClosure {
  public: typedef std::vector<v8::Handle<v8::Value>> Argv;

  private: v8::Persistent<v8::Function> function_;
  private: base::WeakPtr<Runner> runner_;

  public: ScriptClosure(base::WeakPtr<Runner> runner,
                        v8::Handle<v8::Function> function);
  public: ~ScriptClosure();

  public: v8::Isolate* isolate() const;
  public: Runner* runner() const;

  public: void Run(Argv argv);
};

class ScriptCallbackArguments {
  private: ScriptClosure::Argv argv_;
  private: v8::Isolate* isolate_;

  public: ScriptCallbackArguments(v8::Isolate* isolate)
      : isolate_(isolate) {
  }

  public: const ScriptClosure::Argv& argv() const { return argv_; }

  public:void Populate() {
  }

  public: template<typename Arg1, typename... Args> void Populate(
      const Arg1& arg1, const Args&... args) {
    argv_.push_back(gin::Converter<Arg1>::ToV8(isolate_,  arg1));
    Populate(args...);
  }
};

}  // namespace internal

template<typename Sig>
struct ScriptCallback {
};

template<typename... Params>
struct ScriptCallback<base::Callback<void(Params...)>> {
  typedef base::Callback<void(Params...)> Callback;

  static Callback New(base::WeakPtr<Runner> runner,
                      v8::Handle<v8::Function> function) {
    auto closure = new internal::ScriptClosure(runner, function);
    return base::Bind(&ScriptCallback::Run, base::Unretained(closure));
  }

  static void Run(internal::ScriptClosure* closure, Params... params) {
    auto const runner = closure->runner();
    if (!runner)
      return;
    Runner::Scope runner_scope(runner);
    internal::ScriptCallbackArguments args(runner->isolate());
    args.Populate(params...);
    closure->Run(std::move(args.argv()));
    delete closure;
  }
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_closure_h)
