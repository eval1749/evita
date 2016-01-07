// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_MEMORY_SCOPED_REFPTR_H_
#define COMMON_MEMORY_SCOPED_REFPTR_H_

#include <algorithm>

#include "base/logging.h"
#include "base/macros.h"

namespace common {

// This version of scoped_refptr<T> supports C++11 move semantics.
template <class T>
class scoped_refptr {
 public:
  typedef T element_type;

  // For class Foo : public Bar {...}
  // scoped_ptr<Foo> foo;
  // scoped_ptr<Bar> bar(foo);
  template <class U>
  scoped_refptr(const scoped_refptr<U>& other)
      : ptr_(other.get()) {
    if (ptr_)
      ptr_->AddRef();
  }

  scoped_refptr(const scoped_refptr<T>& other) : ptr_(other.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }

  scoped_refptr(scoped_refptr<T>&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  explicit scoped_refptr(T* ptr) : ptr_(ptr) {
    if (ptr_)
      ptr_->AddRef();
  }

  scoped_refptr() : ptr_(nullptr) {}

  ~scoped_refptr() {
    if (ptr_)
      ptr_->Release();
  }

  operator T*() const { return ptr_; }

  T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  scoped_refptr<T>& operator=(const scoped_refptr<T>& other) {
    if (ptr_)
      ptr_->Release();
    ptr_ = other.ptr_;
    if (ptr_)
      ptr_->AddRef();
    return *this;
  }

  scoped_refptr<T>& operator=(scoped_refptr<T>&& other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  scoped_refptr<T>& operator=(T* new_ptr) {
    if (new_ptr)
      new_ptr->AddRef();
    auto const old_ptr = ptr_;
    ptr_ = new_ptr;
    if (old_ptr)
      old_ptr->Release();
    return *this;
  }

  bool operator==(const scoped_refptr<T>& other) const {
    return ptr_ == other.ptr_;
  }

  bool operator==(T* ptr) const { return ptr_ == ptr; }

  bool operator!=(const scoped_refptr<T>& other) const {
    return ptr_ != other.ptr_;
  }

  bool operator!=(T* ptr) const { return ptr_ != ptr; }

  T* get() const { return ptr_; }

  T* release() {
    auto const ret_val = ptr_;
    ptr_ = nullptr;
    return ret_val;
  }

  void swap(scoped_refptr<T>& other) { swap(&other.ptr_); }

  void swap(T** ptr_ptr) {
    T* temp = ptr_;
    ptr_ = *ptr_ptr;
    *ptr_ptr = temp;
  }

 private:
  T* ptr_;
};

// Helper function to write:
//  auto foo = common::make_scoped_refptr(new Foo())
template <class T>
scoped_refptr<T> make_scoped_refptr(T* ptr) {
  return scoped_refptr<T>(ptr);
}

}  // namespace common

#endif  // COMMON_MEMORY_SCOPED_REFPTR_H_
