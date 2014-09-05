// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/per_isolate_data.h"

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/message_loop/message_loop_proxy.h"
#pragma warning(pop)
#include "evita/v8_glue/gin_embedders.h"
#include "evita/v8_glue/v8.h"

namespace v8_glue {

PerIsolateData::PerIsolateData(v8::Isolate* isolate)
    : construct_mode_(kCreateNewObject),
      current_runner_(nullptr),
      isolate_(isolate),
      message_loop_proxy_(base::MessageLoopProxy::current()) {
  isolate_->SetData(gin::kEmbedderEvita, this);
}

PerIsolateData::~PerIsolateData() {
}

PerIsolateData* PerIsolateData::From(v8::Isolate* isolate) {
  return static_cast<PerIsolateData*>(isolate->GetData(gin::kEmbedderEvita));
}

}  // namespace v8_glue
