// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GINX_SCRIPTABLE_H_
#define EVITA_GINX_SCRIPTABLE_H_

#include <type_traits>

#include "base/logging.h"
#include "base/macros.h"
#include "evita/gc/collectable.h"
#include "evita/ginx/object_template_builder.h"
#include "evita/ginx/wrapper_info.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

namespace ginx {

namespace internal {

void* FromV8Impl(v8::Isolate* isolate,
                 v8::Local<v8::Value> value,
                 WrapperInfo* info);

}  // namespace internal

// Note: The |AbstractScriptable| class was called |AbstractScriptWrappable|.
// Although, it is too long to fit in line. So, we renamed shorter name.
class AbstractScriptable : public gc::Collectable<AbstractScriptable> {
 public:
  bool is_instance_of(const WrapperInfo* wrapper_info);
  bool has_script_reference() const { return !wrapper_.IsEmpty(); }

  virtual WrapperInfo* wrapper_info() const = 0;

  // For |new ClassName()|. |Scriptable<T>| is create after wrapper.
  void Bind(v8::Isolate* isolate, v8::Local<v8::Object> wrapper);

  // Get wrapper for existing |Scriptable<T>|.
  v8::Local<v8::Object> GetWrapper(v8::Isolate* isolate) const;

 protected:
  AbstractScriptable();
  ~AbstractScriptable() override;

 private:
  static void WeakCallback(
      const v8::WeakCallbackInfo<AbstractScriptable>& data);

  mutable v8::Global<v8::Object> wrapper_;  // Weak

  DISALLOW_COPY_AND_ASSIGN(AbstractScriptable);
};

template <typename T, typename B = AbstractScriptable>
class Scriptable : public B {
 public:
  template <typename T>
  T* as() {
    return is<T>() ? static_cast<T*>(this) : nullptr;
  }
  template <typename T>
  const T* as() const {
    return is<T>() ? static_cast<const T*>(this) : nullptr;
  }
  template <typename T>
  bool is() const {
    return wrapper_info()->is_descendant_or_self_of(T::static_wrapper_info());
  }

  // Expose as public function for logging. I'm not sure other usages of
  // |AbstractScriptable::wrapper_info()|.
  WrapperInfo* wrapper_info() const override {
    return T::static_wrapper_info();
  }

 protected:
  using ScriptableBase = Scriptable<T, B>;

  template <typename... Params>
  explicit Scriptable(Params&&... params) : B(params...) {}
  virtual ~Scriptable() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Scriptable);
};

}  // namespace ginx

namespace gin {
template <typename T>
struct Converter<
    T*,
    typename std::enable_if<
        std::is_convertible<T*, ginx::AbstractScriptable*>::value>::type> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, T* val) {
    return val->GetWrapper(isolate);
  }

  static bool FromV8(v8::Isolate* isolate, v8::Local<v8::Value> val, T** out) {
    auto const wrapper_info = T::static_wrapper_info();
    *out = static_cast<T*>(static_cast<ginx::AbstractScriptable*>(
        ginx::internal::FromV8Impl(isolate, val, wrapper_info)));
    return *out != nullptr;
  }
};
}  // namespace gin

#define DECLARE_SCRIPTABLE_OBJECT(name)            \
  DECLARE_GC_COLLECTABLE_OBJECT(name)              \
                                                   \
 public:                                           \
  static ginx::WrapperInfo* static_wrapper_info(); \
  static const char* scriptable_class_name() { return #name; }

#define DEFINE_SCRIPTABLE_OBJECT(name, wrapper_info_name)   \
  ginx::WrapperInfo* name::static_wrapper_info() {          \
    CR_DEFINE_STATIC_LOCAL(wrapper_info_name, wrapper_info, \
                           (scriptable_class_name()));      \
    return &wrapper_info;                                   \
  }

#endif  // EVITA_GINX_SCRIPTABLE_H_
