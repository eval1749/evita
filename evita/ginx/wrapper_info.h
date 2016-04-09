// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_GINX_WRAPPER_INFO_H_
#define EVITA_GINX_WRAPPER_INFO_H_

#include <type_traits>

#include "evita/ginx/gin_embedders.h"
#include "evita/ginx/object_template_builder.h"
BEGIN_V8_INCLUDE
#include "gin/public/wrapper_info.h"
END_V8_INCLUDE

namespace ginx {

class AbstractScriptable;

//////////////////////////////////////////////////////////////////////
//
// WrapperInfo
//
// Note: The |WrapperInfo| class was called |ScriptWrapperInfo|. Although, it
// is too long to fit in line. So, we renamed shorter name.
class WrapperInfo {
 protected:
  typedef ginx::AbstractScriptable AbstractScriptable;

 public:
  virtual ~WrapperInfo() = default;

  const char* class_name() const { return class_name_; }
  gin::GinEmbedder embedder() const { return embedder_; }
  virtual WrapperInfo* inherit_from() const;
  bool is_descendant_or_self_of(const WrapperInfo* other) const;
  gin::WrapperInfo* gin_wrapper_info() {
    return const_cast<gin::WrapperInfo*>(
        reinterpret_cast<const gin::WrapperInfo*>(&embedder_));
  }

  static WrapperInfo* From(v8::Local<v8::Object> object);
  v8::Local<v8::FunctionTemplate> GetOrCreateConstructorTemplate(
      v8::Isolate* isolate);
  v8::Local<v8::ObjectTemplate> GetOrCreateInstanceTemplate(
      v8::Isolate* isolate);
  v8::Local<v8::FunctionTemplate> Install(v8::Isolate* isolate,
                                          v8::Local<v8::ObjectTemplate> global);

 protected:
  explicit WrapperInfo(const char* class_name);

  virtual v8::Local<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> templ);

  virtual v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
      v8::Isolate* isolate);
  v8::Local<v8::ObjectTemplate> CreateInstanceTemplate(v8::Isolate* isolate);

 private:
  gin::GinEmbedder const embedder_;
  const char* const class_name_;

  DISALLOW_COPY_AND_ASSIGN(WrapperInfo);
};

//////////////////////////////////////////////////////////////////////
//
// Installer
//
template <typename T>
class Installer final {
 public:
  static v8::Local<v8::FunctionTemplate> Run(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> global) {
    return T::static_wrapper_info()->Install(isolate, global);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Installer);
};

//////////////////////////////////////////////////////////////////////
//
// DerivedWrapperInfo
//
template <typename Derived, typename Base>
class DerivedWrapperInfo : public WrapperInfo {
  static_assert(std::is_base_of<Base, Derived>::value, "Invalid inheritance");

 protected:
  typedef DerivedWrapperInfo<Derived, Base> BaseClass;

  explicit DerivedWrapperInfo(const char* class_name)
      : WrapperInfo(class_name) {}

  ~DerivedWrapperInfo() override = default;

 private:
  ginx::WrapperInfo* inherit_from() const override {
    return Base::static_wrapper_info();
  }

  DISALLOW_COPY_AND_ASSIGN(DerivedWrapperInfo);
};

// TODO(eval1749): Workaround for dom::Editor::MessageBox
#undef MessageBox

}  // namespace ginx

#endif  // EVITA_GINX_WRAPPER_INFO_H_
