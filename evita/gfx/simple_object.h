// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_SIMPLE_OBJECT_H_
#define EVITA_GFX_SIMPLE_OBJECT_H_

#include "base/macros.h"
#include "common/win/scoped_comptr.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// Object
//
class Object {
 protected:
  Object();
  ~Object();

 private:
  DISALLOW_COPY_AND_ASSIGN(Object);
};

//////////////////////////////////////////////////////////////////////
//
// SimpleObject_
//
template <class T>
class SimpleObject_ : public Object {
 public:
  operator T*() const { return ptr_; }
  T* operator->() const { return ptr_; }

 protected:
  explicit SimpleObject_(T* ptr) : ptr_(ptr) {}
  explicit SimpleObject_(common::ComPtr<T>&& ptr) : ptr_(std::move(ptr)) {}

 private:
  const common::ComPtr<T> ptr_;

  DISALLOW_COPY_AND_ASSIGN(SimpleObject_);
};

}  // namespace gfx

#endif  // EVITA_GFX_SIMPLE_OBJECT_H_
