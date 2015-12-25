// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GC_LOCAL_H_
#define EVITA_GC_LOCAL_H_

#include "base/macros.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template <typename T>
class Local final {
 public:
  explicit Local(T* ptr) : ptr_(ptr) {}
  Local(const Local& other) : ptr_(other.ptr_) {}
  Local(Local&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }
  Local() : ptr_(nullptr) {}
  ~Local() = default;

  operator T*() const { return ptr_; }
  T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  Local& operator=(const Local& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  Local& operator=(Local&& other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  Local& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  bool operator==(const Local& other) const { return ptr_ == other.ptr_; }

  bool operator==(T* ptr) const { return ptr_ == ptr; }

  bool operator!=(const Local& other) const { return ptr_ != other.ptr_; }

  bool operator!=(T* ptr) const { return ptr_ != ptr; }

  T* get() const { return ptr_; }

 private:
  T* ptr_;
};

// Handy utility for creating a gc::Local<T> out of a T* explicitly without
// having to retype all the template arguments
template <typename T>
Local<T> MakeLocal(T* ptr) {
  return Local<T>(ptr);
}

}  // namespace gc

#endif  // EVITA_GC_LOCAL_H_
