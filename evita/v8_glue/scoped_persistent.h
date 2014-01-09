// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_scoped_persistent_h)
#define INCLUDE_evita_v8_glue_scoped_persistent_h

#include "evita/v8_glue/v8.h"

namespace v8_glue {

template<typename ObjectType>
class ScopedPersistent {
  private: v8::Persistent<ObjectType> handle_;

  public: ScopedPersistent() {
  }

  public: ScopedPersistent(v8::Isolate* isolate,
                           v8::Handle<ObjectType> handle)
      : handle_(isolate, handle) {
  }

  public: ~ScopedPersistent() {
    handle_.Reset();
  }

  public: v8::Local<ObjectType> NewLocal(v8::Isolate* isolate) const {
    return v8::Local<ObjectType>::New(isolate, handle_);
  }

  public: void Reset(v8::Isolate* isolate, v8::Handle<ObjectType> value) {
    handle_.Reset(isolate, value);
  }

  DISALLOW_COPY_AND_ASSIGN(ScopedPersistent);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_scoped_persistent_h)
