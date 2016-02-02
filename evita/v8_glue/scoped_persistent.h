// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_V8_GLUE_SCOPED_PERSISTENT_H_
#define EVITA_V8_GLUE_SCOPED_PERSISTENT_H_

#include "evita/v8_glue/v8.h"

namespace v8_glue {

template <typename ObjectType>
class ScopedPersistent final {
 public:
  ScopedPersistent(v8::Isolate* isolate, v8::Local<ObjectType> handle)
      : handle_(isolate, handle) {}

  ScopedPersistent() {}

  ~ScopedPersistent() { handle_.Reset(); }

  const v8::Persistent<ObjectType>* operator->() const { return &handle_; }
  v8::Persistent<ObjectType>* operator->() { return &handle_; }

  bool operator==(v8::Local<ObjectType> handle) const {
    return handle == handle_;
  }

  bool operator!=(v8::Local<ObjectType> handle) const {
    return handle != handle_;
  }

  bool IsEmpty() const { return handle_.IsEmpty(); }

  v8::Local<ObjectType> NewLocal(v8::Isolate* isolate) const {
    return v8::Local<ObjectType>::New(isolate, handle_);
  }

  void Reset(v8::Isolate* isolate, v8::Local<ObjectType> handle) {
    handle_.Reset(isolate, handle);
  }

  void Reset() { handle_.Reset(); }

 private:
  v8::Persistent<ObjectType> handle_;

  DISALLOW_COPY_AND_ASSIGN(ScopedPersistent);
};

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_SCOPED_PERSISTENT_H_
