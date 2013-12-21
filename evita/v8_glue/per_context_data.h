// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_per_context_data_h)
#define INCLUDE_evita_v8_glue_per_context_data_h

#include "evita/v8_glue/v8.h"

namespace v8_glue {

class PerContextData {
  public: explicit PerContextData(v8::Handle<v8::Context> context);
  public: ~PerContextData();

  public: void Detach(v8::Handle<v8::Context> context);

  DISALLOW_COPY_AND_ASSIGN(PerContextData);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_per_context_data_h)
