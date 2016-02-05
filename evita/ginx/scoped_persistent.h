// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GINX_SCOPED_PERSISTENT_H_
#define EVITA_GINX_SCOPED_PERSISTENT_H_

#include "base/macros.h"
#include "evita/ginx/v8.h"

namespace ginx {

//////////////////////////////////////////////////////////////////////
//
// ScopedPersistent
//
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

}  // namespace ginx

#endif  // EVITA_GINX_SCOPED_PERSISTENT_H_
