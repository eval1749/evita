// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_script_wrapper_info_h)
#define INCLUDE_evita_v8_glue_script_wrapper_info_h

#include "evita/v8_glue/gin_embedders.h"
BEGIN_V8_INCLUDE
#include "gin/public/wrapper_info.h"
END_V8_INCLUDE

namespace v8_glue {

class ScriptWrapperInfo {
  // |embedder_| must be a first member to be compatible with gin::WrapperInfo.
  public: gin::GinEmbedder const embedder_;
  private: const char* const class_name_;

  public: ScriptWrapperInfo(const char* class_name)
      : embedder_(kEmbedderEvita),
        class_name_(class_name) {
  }
  public: ~ScriptWrapperInfo() = default;

  public: const char* class_name() const { return class_name_; }
  public: gin::WrapperInfo* gin_wrapper_info() {
    return reinterpret_cast<gin::WrapperInfo*>(this);
  }

  DISALLOW_COPY_AND_ASSIGN(ScriptWrapperInfo);
};

static_assert(
    offsetof(ScriptWrapperInfo, embedder_) ==
        offsetof(gin::WrapperInfo, embedder),
    "Layout of ScriptWrapperInfo must be matched to gin::WrapperInfo");

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_script_wrapper_info_h)
