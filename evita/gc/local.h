// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_local_h)
#define INCLUDE_evita_gc_local_h

#include "base/basictypes.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template<typename T>
class Local {
  private: T* ptr_;

  public: explicit Local(T* ptr) : ptr_(ptr) {
  }
  public: Local(const Local& other) : ptr_(other.ptr_) {
  }
  public: Local(Local&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  public: Local() : ptr_(nullptr) {
  }
  public: ~Local() = default;

  public: operator T*() const { return ptr_; }
  public: T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  public: Local& operator=(const Local& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  public: Local& operator=(Local&& other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  public: Local& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  public: bool operator==(const Local& other) const {
    return ptr_ == other.ptr_;
  }

  public: bool operator==(T* ptr) const {
    return ptr_ == ptr;
  }

  public: bool operator!=(const Local& other) const {
    return ptr_ != other.ptr_;
  }

  public: bool operator!=(T* ptr) const {
    return ptr_ != ptr;
  }

  public: T* get() const { return ptr_; }
};

// Handy utility for creating a gc::Local<T> out of a T* explicitly without
// having to retype all the template arguments
template<typename T>
Local<T> MakeLocal(T* ptr) {
  return Local<T>(ptr);
}

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_local_h)
