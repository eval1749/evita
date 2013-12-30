// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/wrapper_info.h"

namespace v8_glue {

WrapperInfo::WrapperInfo(const char* class_name,
                         WrapperInfo* inherit_from,
                         const char* singleton_name)
    : embedder_(gin::kEmbedderEvita),
      class_name_(class_name),
      inherit_from_(inherit_from),
      singleton_name_(singleton_name) {
}

WrapperInfo::WrapperInfo(const char* class_name,
                         WrapperInfo* inherit_from)
    : WrapperInfo(class_name, inherit_from, nullptr) {
}

WrapperInfo::WrapperInfo(const char* class_name, const char* singleton_name)
    : WrapperInfo(class_name, nullptr, singleton_name) {
}

WrapperInfo::WrapperInfo(const char* class_name)
    : WrapperInfo(class_name, nullptr, nullptr) {
}

WrapperInfo* WrapperInfo::From(v8::Handle<v8::Object> object) {
  if (object->InternalFieldCount() != gin::kNumberOfInternalFields)
    return nullptr;
  auto const info = static_cast<WrapperInfo*>(
      object->GetAlignedPointerFromInternalField(gin::kWrapperInfoIndex));
  return info->embedder() == gin::kEmbedderEvita ? info : nullptr;
}

}  // namespace v8_glue
