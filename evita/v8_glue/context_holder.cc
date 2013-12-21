// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/context_holder.h"

#include "base/logging.h"
#include "evita/v8_glue/per_context_data.h"

namespace v8_glue {

ContextHolder::ContextHolder(v8::Isolate* isolate)
    : isolate_(isolate) {
}

ContextHolder::~ContextHolder() {
  v8::HandleScope handle_scope(isolate_);
  v8::Handle<v8::Context> context = this->context();
  data_->Detach(context);
  data_.reset();
}

void ContextHolder::SetContext(v8::Handle<v8::Context> context) {
  DCHECK(context_.IsEmpty());
  context_.Reset(isolate_, context);
  data_.reset(new PerContextData(context));
}

}  // namespace v8_glue
