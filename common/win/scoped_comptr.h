// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_com_ptr_h)
#define INCLUDE_common_com_ptr_h

namespace common {

#if defined(_DEBUG)
#define COM_VERIFY(expr) { \
  auto const macro_hr = (expr); \
  if (FAILED(macro_hr)) \
    Debugger::Fail("hr=%08X\r\n%s\r\n", macro_hr, #expr); \
}
#else
#define COM_VERIFY(expr) { \
  auto const macro_hr = (expr); \
  if (FAILED(macro_hr)) \
    Debugger::Fail("hr=%08X\r\n%s\r\n", macro_hr, #expr); \
}
#endif

template<class T> class ComPtr {
  private: T* ptr_;
  public: explicit ComPtr(T* ptr = nullptr) : ptr_(ptr) {}
  public: explicit ComPtr(T& ptr) : ptr_(&ptr) {}
  public: ComPtr(ComPtr& other) : ptr_(other.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }
  public: ComPtr(ComPtr&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  public: ~ComPtr() {
    if (ptr_)
      ptr_->Release();
  }
  public: operator T*() const { return ptr_; }
  public: operator bool() const { return ptr_; }
  public: T* operator->() const { return ptr_; }
  public: T** operator&() { return &ptr_; }
  public: bool operator!() const { return !ptr_; }

  public: bool operator==(const ComPtr& other) const {
    return ptr_ == other.ptr_;
  }

  public: bool operator==(T* other) const {
    return ptr_ == other;
  }

  public: bool operator!=(const ComPtr& other) const {
    return ptr_ != other.ptr_;
  }

  public: bool operator!=(T* other) const {
    return ptr_ != other;
  }

  public: ComPtr& operator=(const ComPtr& other) {
    ptr_ = other.ptr_;
    if (ptr_)
      ptr_->AddRef();
  }

  public: ComPtr& operator=(ComPtr&& other) {
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    if (ptr_)
      ptr_->AddRef();
  }

  public: IUnknown** locationUnknown() {
    return reinterpret_cast<IUnknown**>(&ptr_);
  }
};

class ComInit {
  public: ComInit() { COM_VERIFY(::CoInitialize(nullptr)); }
  public: ~ComInit() { ::CoUninitialize(); }
};

} // namespace common

#endif //!defined(INCLUDE_common_com_ptr_h)
