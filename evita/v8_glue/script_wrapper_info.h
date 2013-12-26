// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_script_wrapper_info_h)
#define INCLUDE_evita_v8_glue_script_wrapper_info_h

BEGIN_V8_INCLUDE
#include "gin/public/wrapper_info.h"
END_V8_INCLUDE

#define kEmbedderEvita kEmbedderBlink

namespace v8_glue {

typedef gin::WrapperInfo ScriptWrapperInfo;

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_script_wrapper_info_h)
