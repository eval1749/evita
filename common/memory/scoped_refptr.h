// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_scoped_refptr_h)
#define INCLUDE_common_scoped_refptr_h

#include "base/basictypes.h"
#include "base/logging.h"

namespace common {

// This version of scoped_refptr<T> supports C++11 move semantics.
template<class T>
class scoped_refptr {
  public: typedef T element_type;

  private: T* ptr_;

  // For class Foo : public Bar {...}
  // scoped_ptr<Foo> foo;
  // scoped_ptr<Bar> bar(foo);
  public: template<class U> scoped_refptr(const scoped_refptr<U>& other)
      : ptr_(other.get()) {
    if (ptr_)
      ptr_->AddRef();
  }

  public: scoped_refptr(const scoped_refptr<T>& other) : ptr_(other.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }

  public: scoped_refptr(scoped_refptr<T>&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  public: scoped_refptr(T* ptr) : ptr_(ptr) {
    if (ptr_)
      ptr_->AddRef();
  }

  public: scoped_refptr() : ptr_(nullptr) {
  }

  public: ~scoped_refptr() {
    if (ptr_)
      ptr_->Release();
  }

  public: operator T*() const { return ptr_; }
  public: T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }
  public: scoped_refptr<T>& operator=(const scoped_refptr<T>& other) {
    if (ptr_)
      ptr_->Release();
    ptr_ = other.ptr_;
    if (ptr_)
      ptr_->AddRef();
    return *this;
  }

  public: scoped_refptr<T>& operator=(scoped_refptr<T>&& other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  public: scoped_refptr<T>& operator=(T* new_ptr) {
    if (new_ptr)
      new_ptr->AddRef();
    auto const old_ptr = ptr_;
    ptr_ = new_ptr;
    if (old_ptr)
      old_ptr->Release();
    return *this;
  }

  public: bool operator==(const scoped_refptr<T>& other) const {
    return ptr_ == other.ptr_;
  }

  public: bool operator==(T* ptr) const {
    return ptr_ == ptr;
  }

  public: bool operator!=(const scoped_refptr<T>& other) const {
    return ptr_ != other.ptr_;
  }

  public: bool operator!=(T* ptr) const {
    return ptr_ != ptr;
  }

  public: T* get() const { return ptr_; }
  public: T* release() {
    auto const ret_val = ptr_;
    ptr_ = nullptr;
    return ret_val;
  }
  public: void swap(scoped_refptr<T>& other) { swap(&other.ptr_); }
  public: void swap(T** ptr_ptr) {
    T* temp = ptr_;
    ptr_ = *ptr_ptr;
    *ptr_ptr = temp;
  }
};

// Helper function to write:
//  auto foo = common::make_scoped_refptr(new Foo())
template<class T>
scoped_refptr<T> make_scoped_refptr(T* ptr) {
  return scoped_refptr<T>(ptr);
}

}  // namespace common

#endif //!defined(INCLUDE_common_scoped_refptr_h)
