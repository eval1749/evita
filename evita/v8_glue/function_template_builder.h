// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_function_template_builder)
#define INCLUDE_evita_v8_glue_function_template_builder

#include "evita/v8_glue/function_template.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

// Based on gin/object_template_builder.h

namespace v8_glue {

namespace internal {

// Base template - used only for non-member function pointers. Other types
// either go to one of the below specializations, or go here and fail to compile
// because of base::Bind().
template<typename T, typename Enable = void>
struct CallbackTraits {
  static v8::Handle<v8::FunctionTemplate> CreateTemplate(
      v8::Isolate* isolate,
      T callback) {
    return gin::CreateFunctionTemplate(isolate, base::Bind(callback));
  }
};

// Specialization for base::Callback.
template<typename T>
struct CallbackTraits<base::Callback<T> > {
  static v8::Handle<v8::FunctionTemplate> CreateTemplate(
      v8::Isolate* isolate,
      const base::Callback<T>& callback) {
    return gin::CreateFunctionTemplate(isolate, callback);
  }
};

// Specialization for member function pointers. We need to handle this case
// specially because the first parameter for callbacks to MFP should typically
// come from the the JavaScript "this" object the function was called on, not
// from the first normal parameter.
template<typename T>
struct CallbackTraits<T, typename base::enable_if<
                           base::is_member_function_pointer<T>::value>::type> {
  static v8::Handle<v8::FunctionTemplate> CreateTemplate(
      v8::Isolate* isolate, T callback) {
    return gin::CreateFunctionTemplate(
        isolate, base::Bind(callback),
        gin::HolderIsFirstArgument);
  }
};

// This specialization allows people to construct function templates directly if
// they need to do fancier stuff.
template<>
struct CallbackTraits<v8::Handle<v8::FunctionTemplate> > {
  static v8::Handle<v8::FunctionTemplate> CreateTemplate(
      v8::Handle<v8::FunctionTemplate> templ) {
    return templ;
  }
};

}  // namespace internal

// FunctionTemplateBuilder provides a handy interface to creating
// v8::ObjectTemplate instances with various sorts of properties.
class FunctionTemplateBuilder {
  public: FunctionTemplateBuilder(v8::Isolate* isolate,
      v8::Handle<v8::FunctionTemplate> function_template);
  public: explicit FunctionTemplateBuilder(v8::Isolate* isolate);
  public: ~FunctionTemplateBuilder();

  // It's against Google C++ style to return a non-const ref, but we take some
  // poetic license here in order that all calls to Set() can be via the '.'
  // operator and line up nicely.
  public: template<typename T>
  FunctionTemplateBuilder& SetValue(const base::StringPiece& name, T val) {
    return SetImpl(name, gin::ConvertToV8(isolate_, val));
  }

  // In the following methods, T and U can be function pointer, member function
  // pointer, base::Callback, or v8::FunctionTemplate. Most clients will want to
  // use one of the first two options. Also see gin::CreateFunctionTemplate()
  // for creating raw function templates.
  public: template<typename T>
  FunctionTemplateBuilder& SetMethod(const base::StringPiece& name,
                                     const T& callback) {
    return SetImpl(name, internal::CallbackTraits<T>::CreateTemplate(
        isolate_, callback));
  }
  public: template<typename T>
  FunctionTemplateBuilder& SetProperty(const base::StringPiece& name,
                                       const T& getter) {
    return SetPropertyImpl(name,
                           internal::CallbackTraits<T>::CreateTemplate(
                              isolate_, getter),
                           v8::Local<v8::FunctionTemplate>());
  }
  public: template<typename T, typename U>
  FunctionTemplateBuilder& SetProperty(const base::StringPiece& name,
                                     const T& getter, const U& setter) {
    return SetPropertyImpl(
        name,
        internal::CallbackTraits<T>::CreateTemplate(isolate_, getter),
        internal::CallbackTraits<U>::CreateTemplate(isolate_, setter));
  }

  public: v8::Local<v8::FunctionTemplate> Build();

  private: FunctionTemplateBuilder& SetImpl(const base::StringPiece& name,
                                           v8::Handle<v8::Data> val);
  private: FunctionTemplateBuilder& SetPropertyImpl(
      const base::StringPiece& name, v8::Handle<v8::FunctionTemplate> getter,
      v8::Handle<v8::FunctionTemplate> setter);

  private: v8::Isolate* isolate_;

  // FunctionTemplateBuilder should only be used on the stack.
  private: v8::Local<v8::FunctionTemplate> template_;
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_function_template_builder)
