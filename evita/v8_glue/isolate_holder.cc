// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/isolate_holder.h"

#include "evita/v8_glue/per_isolate_data.h"

namespace v8_glue {

IsolateHolder::IsolateHolder() : isolate_data_(new PerIsolateData(isolate())) {}

}  // namespace v8_glue
