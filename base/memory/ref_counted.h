// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_ref_counted_h)
#define INCLUDE_base_ref_counted_h

#include "base/logging.h"

namespace base {

template<class T>
class RefCounted {
  private: mutable int ref_count_;
  protected: RefCounted() : ref_count_(0) {}
  public: ~RefCounted() {
    DCHECK(!ref_count_);
  }
  public: int ref_count() const { return ref_count_; }
  public: void AddRef() const { ++ref_count_; }
  public: bool Release() const {
    DCHECK(ref_count_ >= 1);
    --ref_count_;
    if (ref_count_)
      return false;
    delete static_cast<T*>(const_cast<RefCounted*>(this));
    return true;
  }
};

template<class T> class scoped_refptr {
  private: T* data_;
  public: scoped_refptr() : data_(nullptr) {
  }
  public: scoped_refptr(T& data) : data_(&data) {
    data->AddRef();
  }
  public: scoped_refptr(T* data) : data_(data) {
    if (data)
      data_->AddRef();
  }
  public: scoped_refptr(const scoped_refptr& ptr) : data_(ptr.data_) {
    if (data_)
      data_->AddRef();
  }
  public: scoped_refptr(scoped_refptr&& ptr) : data_(ptr.data_) {
    ptr.data_ = nullptr;
  }
  public: ~scoped_refptr() {
    if (data_)
      data_->Release();
  }

  public: operator T*() const { return data_; }
  public: operator bool() const { return data_; }
  public: T* operator->() const { return data_; }
  public: T& operator*() const { return *data_; }

  public: scoped_refptr& operator=(const scoped_refptr& other) {
    reset(other.data_);
  }

  public: scoped_refptr& operator=(scoped_refptr&& other) {
    if (data_)
      data_->Release();
    data_ = other.data_;
    other.data_ = nullptr;
    if (data_)
      data_->AddRef();
    return *this;
  }

  public: T* get() const { return data_; }

  public: void reset(T* new_data = nullptr) {
    if (data_)
      data_->Release();
    data_ = new_data;
    if (data_)
      data_->AddRef();
  }
};

using base::scoped_refptr;

} // namespace base

#endif //!defined(INCLUDE_base_ref_counted_h)
