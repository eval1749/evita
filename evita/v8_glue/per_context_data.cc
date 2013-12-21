// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/per_context_data.h"

namespace v8_glue {

PerContextData::PerContextData(v8::Handle<v8::Context>) {
}

PerContextData::~PerContextData() {
}

void PerContextData::Detach(v8::Handle<v8::Context>) {
}

}  // namespace v8_glue
