// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_member_h)
#define INCLUDE_evita_gc_member_h

#include "base/basictypes.h"
#include "base/logging.h"

namespace gc {

// Note: At time time, December 2013, it is just marker for root set.
template<typename T>
class Member {
  private: T* ptr_;

  public: explicit Member(const Member& other) : ptr_(other.ptr_) {
  }
  public: explicit Member(T* ptr) : ptr_(ptr) {
  }
  public: Member() : ptr_(nullptr) {
  }
  public: ~Member() = default;

  public: operator T*() const { return ptr_; }
  public: T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

  public: Member& operator=(const Member& other) {
    ptr_ = other.ptr_;
    return *this;
  }

  public: Member& operator=(T* ptr) {
    ptr_ = ptr;
    return *this;
  }

  public: bool operator==(const Member& other) const {
    return ptr_ == other.ptr_;
  }

  public: bool operator==(T* ptr) const {
    return ptr_ == ptr;
  }

  public: bool operator!=(const Member& other) const {
    return ptr_ != other.ptr_;
  }

  public: bool operator!=(T* ptr) const {
    return ptr_ != ptr;
  }

  public: T* get() const { return ptr_; }
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_member_h)
