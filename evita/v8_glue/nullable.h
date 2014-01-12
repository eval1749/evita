// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_nullable_h)
#define INCLUDE_evita_v8_glue_nullable_h

#include "base/logging.h"
#include "base/template_util.h"

namespace v8_glue {
class AbstractScriptable;

template<typename T>
class Nullable {
  private: T* ptr_;
  public: Nullable(const Nullable& other) : ptr_(other.ptr_) {}
  public: Nullable(T* ptr) : ptr_(ptr) {}
  public: operator T*() const { return ptr_; }
  public: T* operator->() const { DCHECK(ptr_); return ptr_; }
};
}  // namespace v8_glue

namespace gin {
template<typename T>
struct Converter<v8_glue::Nullable<T>, typename base::enable_if<
    base::is_convertible<T*,
        v8_glue::AbstractScriptable*>::value>::type> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T* val) {
    if (val)
      return val->GetWrapper(isolate);
    return v8::Null(isolate);
  }

  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val, T** out) {
    if (val->IsNull()) {
      *out = nullptr;
      return true;
    }
    auto const wrapper_info = T::static_wrapper_info();
    *out = static_cast<T*>(static_cast<v8_glue::AbstractScriptable*>(
        v8_glue::internal::FromV8Impl(isolate, val, wrapper_info)));
    return *out;
  }
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_v8_glue_nullable_h)
