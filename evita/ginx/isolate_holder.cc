// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ginx/isolate_holder.h"

#include "evita/ginx/per_isolate_data.h"

namespace ginx {

IsolateHolder::IsolateHolder() : isolate_data_(new PerIsolateData(isolate())) {}

}  // namespace ginx
