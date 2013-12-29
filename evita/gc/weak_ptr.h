// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_weak_ptr_h)
#define INCLUDE_evita_gc_weak_ptr_h

#include "base/basictypes.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template<typename T>
class WeakPtr {
  private: T* ptr_;

  public: explicit WeakPtr(const WeakPtr& other) : ptr_(other.ptr_) {
  }
  public: explicit WeakPtr(T* ptr) : ptr_(ptr) {
  }
  public: WeakPtr() : ptr_(nullptr) {
  }
  public: ~WeakPtr() = default;

  public: operator T*() const { return ptr_; }
  public: T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  public: WeakPtr& operator=(const WeakPtr& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  public: WeakPtr& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  public: bool operator==(const WeakPtr& other) const {
    return ptr_ == other.ptr_;
  }

  public: bool operator==(T* ptr) const {
    return ptr_ == ptr;
  }

  public: bool operator!=(const WeakPtr& other) const {
    return ptr_ != other.ptr_;
  }

  public: bool operator!=(T* ptr) const {
    return ptr_ != ptr;
  }

  public: T* get() const { return ptr_; }
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_weak_ptr_h)
