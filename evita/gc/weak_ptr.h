// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GC_WEAK_PTR_H_
#define EVITA_GC_WEAK_PTR_H_

#include "base/macros.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template <typename T>
class WeakPtr final {
 public:
  explicit WeakPtr(const WeakPtr& other) : ptr_(other.ptr_) {}
  explicit WeakPtr(T* ptr) : ptr_(ptr) {}
  WeakPtr() : ptr_(nullptr) {}
  ~WeakPtr() = default;

  operator T*() const { return ptr_; }
  T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  WeakPtr& operator=(const WeakPtr& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  WeakPtr& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  bool operator==(const WeakPtr& other) const { return ptr_ == other.ptr_; }

  bool operator==(T* ptr) const { return ptr_ == ptr; }

  bool operator!=(const WeakPtr& other) const { return ptr_ != other.ptr_; }

  bool operator!=(T* ptr) const { return ptr_ != ptr; }

  T* get() const { return ptr_; }

 private:
  T* ptr_;
};

}  // namespace gc

#endif  // EVITA_GC_WEAK_PTR_H_
