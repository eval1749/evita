// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_WIN_SCOPED_COMPTR_H_
#define COMMON_WIN_SCOPED_COMPTR_H_

#include <unknwn.h>

#include "base/logging.h"
#include "common/win/com_verify.h"

namespace common {

template <class T>
class ComPtr final {
 public:
  explicit ComPtr(T* ptr = nullptr) : ptr_(ptr) {
    if (ptr_)
      ptr_->AddRef();
  }
  explicit ComPtr(T& ptr) : ptr_(&ptr) {}
  ComPtr(const ComPtr& other) : ptr_(other.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }
  ComPtr(ComPtr&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }
  ~ComPtr() { reset(); }

  operator T*() const { return ptr_; }
  explicit operator bool() const { return ptr_; }
  T* operator->() const { return ptr_; }
  T** operator&() {  // NOLINT(runtime/operator)
    DCHECK(!ptr_) << "Leak COM interface";
    return &ptr_;
  }
  bool operator!() const { return !ptr_; }
  bool operator==(const ComPtr& other) const { return ptr_ == other.ptr_; }
  bool operator==(T* other) const { return ptr_ == other; }
  bool operator!=(const ComPtr& other) const { return ptr_ != other.ptr_; }
  bool operator!=(T* other) const { return ptr_ != other; }
  ComPtr& operator=(const ComPtr& other) { reset(other.ptr_); }
  ComPtr& operator=(ComPtr&& other) {
    if (ptr_)
      ptr_->Release();
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }
  ComPtr& operator=(T* ptr) = delete;

  T* get() const { return ptr_; }

  void** location() {
    DCHECK(!ptr_) << "Leak COM interface";
    return reinterpret_cast<void**>(&ptr_);
  }

  IUnknown** locationUnknown() {
    DCHECK(!ptr_) << "Leak COM interface";
    return reinterpret_cast<IUnknown**>(&ptr_);
  }

  T* release() {
    DCHECK(ptr_) << "Give ownership for uninitialized object";
    auto const ret = ptr_;
    ptr_ = nullptr;
    return ret;
  }

  void reset(T* ptr) {
    if (ptr_)
      ptr_->Release();
    ptr_ = ptr;
    if (ptr_)
      ptr_->AddRef();
  }

  void reset() {
    if (ptr_)
      ptr_->Release();
    ptr_ = nullptr;
  }

  HRESULT QueryFrom(IUnknown* other) {
    return other->QueryInterface(__uuidof(T), location());
  }

 private:
  T* ptr_;
};

}  // namespace common

#endif  // COMMON_WIN_SCOPED_COMPTR_H_
