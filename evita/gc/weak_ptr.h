// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GC_WEAK_PTR_H_
#define EVITA_GC_WEAK_PTR_H_

#include "base/basictypes.h"
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
