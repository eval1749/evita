// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/wrapper_info.h"

namespace v8_glue {

WrapperInfo* WrapperInfo::From(v8::Handle<v8::Object> object) {
  if (object->InternalFieldCount() != gin::kNumberOfInternalFields)
    return nullptr;
  auto const info = static_cast<WrapperInfo*>(
      object->GetAlignedPointerFromInternalField(gin::kWrapperInfoIndex));
  return info->embedder() == gin::kEmbedderEvita ? info : nullptr;
}

}  // namespace v8_glue
