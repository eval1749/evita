// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_script_wrappable_h)
#define INCLUDE_evita_v8_glue_script_wrappable_h

#include "evita/v8_glue/script_wrapper_info.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

namespace v8_glue {

class AbstractScriptWrappable {
  private: v8::Persistent<v8::Object> wrapper_; // Weak

  protected: AbstractScriptWrappable() = default;
  protected: virtual ~AbstractScriptWrappable();

  public: bool has_script_reference() const {
    return !wrapper_.IsEmpty();
  }

  protected: virtual const char* wrapper_class_name() const { return nullptr; }
  protected: virtual ScriptWrapperInfo* wrapper_info() const = 0;

  public: virtual v8::Handle<v8::FunctionTemplate>
      GetFunctionTemplate(v8::Isolate* isolate);

  protected: virtual gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate);

  public: v8::Handle<v8::Object> GetWrapper(v8::Isolate* isolate);

  private: static void WeakCallback(
      const v8::WeakCallbackData<v8::Object, AbstractScriptWrappable>& data);

  DISALLOW_COPY_AND_ASSIGN(AbstractScriptWrappable);
};

template<typename T>
class ScriptWrappable : public AbstractScriptWrappable {
  protected: ScriptWrappable() = default;
  protected: virtual ~ScriptWrappable() = default;

  private: ScriptWrapperInfo* wrapper_info() const {
    return &T::kWrapperInfo;
  }

  // A helper function to get ObjecTemplateBuilder from
  // AbstractScriptWrappable to avoid v8_glue::ScriptWrapperInfo<T> prefix.
  protected: gin::ObjectTemplateBuilder GetEmptyObjectTemplateBuilder(
      v8::Isolate* isolate) {
    return AbstractScriptWrappable::GetObjectTemplateBuilder(isolate);
  }

  DISALLOW_COPY_AND_ASSIGN(ScriptWrappable);
};

}  // namespace v8_glue

namespace gin {

// This converter handles any subclass of Wrappable.
template<typename T>
struct Converter<T*, typename base::enable_if<
    base::is_convertible<T*,
        v8_glue::AbstractScriptWrappable*>::value>::type> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, T* val) {
    return val->GetWrapper(isolate);
  }

  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val, T** out) {
    *out = static_cast<T*>(static_cast<v8_glue::AbstractScriptWrappable*>(
        internal::FromV8Impl(isolate, val, &T::kWrapperInfo)));
    return *out;
  }
};

}  // namespace gin

#endif //!defined(INCLUDE_evita_v8_glue_script_wrappable_h)
