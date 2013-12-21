// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_context_holder_h)
#define INCLUDE_evita_v8_glue_context_holder_h

#include "evita/v8_glue/v8.h"
#include <memory>

namespace v8_glue {

class PerContextData;

class ContextHolder {
  private: v8::UniquePersistent<v8::Context> context_;
  private: std::unique_ptr<PerContextData> data_;
  private: v8::Isolate* isolate_;

  public: explicit ContextHolder(v8::Isolate* isolate);
  public: ~ContextHolder();

  public: v8::Isolate& isolate() const { return *isolate_; }

  public: v8::Handle<v8::Context> context() const {
    return v8::Local<v8::Context>::New(isolate_, context_);
  }

  public: void SetContext(v8::Handle<v8::Context> context);

   DISALLOW_COPY_AND_ASSIGN(ContextHolder);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_context_holder_h)
