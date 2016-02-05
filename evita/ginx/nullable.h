// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_GINX_NULLABLE_H_
#define EVITA_GINX_NULLABLE_H_

#include <type_traits>

#include "base/logging.h"
#include "evita/ginx/converter.h"

namespace ginx {
class AbstractScriptable;

template <typename T>
class Nullable final {
 public:
  Nullable(T* ptr) : ptr_(ptr) {}  // NOLINT
  Nullable(const Nullable& other) : ptr_(other.ptr_) {}
  Nullable() : ptr_(nullptr) {}

  operator T*() const { return ptr_; }
  T* operator->() const {
    DCHECK(ptr_);
    return ptr_;
  }

 private:
 private:
  T* ptr_;
};
}  // namespace ginx

namespace gin {
template <typename T>
struct Converter<
    ginx::Nullable<T>,
    typename std::enable_if<
        std::is_convertible<T*, ginx::AbstractScriptable*>::value>::type> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, T* val) {
    if (val)
      return val->GetWrapper(isolate);
    return v8::Null(isolate);
  }

  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     ginx::Nullable<T>* out) {
    if (val->IsNull()) {
      *out = nullptr;
      return true;
    }
    auto const wrapper_info = T::static_wrapper_info();
    *out = static_cast<T*>(static_cast<ginx::AbstractScriptable*>(
        ginx::internal::FromV8Impl(isolate, val, wrapper_info)));
    return *out != nullptr;
  }
};
}  // namespace gin

#endif  // EVITA_GINX_NULLABLE_H_
