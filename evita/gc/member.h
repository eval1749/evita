// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GC_MEMBER_H_
#define EVITA_GC_MEMBER_H_

#include "base/macros.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template <typename T>
class Member final {
 public:
  explicit Member(const Member& other) : ptr_(other.ptr_) {}
  explicit Member(T* ptr) : ptr_(ptr) {}
  Member() : ptr_(nullptr) {}
  ~Member() = default;

  operator T*() const { return ptr_; }
  T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  Member& operator=(const Member& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  Member& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  bool operator==(const Member& other) const { return ptr_ == other.ptr_; }

  bool operator==(T* ptr) const { return ptr_ == ptr; }

  bool operator!=(const Member& other) const { return ptr_ != other.ptr_; }

  bool operator!=(T* ptr) const { return ptr_ != ptr; }

  T* get() const { return ptr_; }

 private:
  T* ptr_;
};

}  // namespace gc

#endif  // EVITA_GC_MEMBER_H_
