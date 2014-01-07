// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_closure_h)
#define INCLUDE_evita_v8_glue_closure_h

#include <vector>
#include <utility>

#include "base/callback.h"
#include "evita/v8_glue/converter.h"

namespace v8_glue {

namespace internal {

class ScriptClosure {
  public: typedef std::vector<v8::Handle<v8::Value>> Argv;

  private: v8::Persistent<v8::Function> function_;
  private: v8::Isolate* isolate_;

  public: ScriptClosure(v8::Isolate* isolate,
                        v8::Handle<v8::Function> function);
  public: ~ScriptClosure();

  public: v8::Isolate* isolate() const { return isolate_;}

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

  static Callback New(v8::Isolate* isolate,
                      v8::Handle<v8::Function> function) {
    auto closure = new internal::ScriptClosure(isolate, function);
    return base::Bind(&ScriptCallback::Run, base::Unretained(closure));
  }

  static void Run(internal::ScriptClosure* closure, Params... params) {
    internal::ScriptCallbackArguments args(closure->isolate());
    args.Populate(params...);
    closure->Run(std::move(args.argv()));
    delete closure;
  }
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_closure_h)
