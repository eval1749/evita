// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/per_isolate_data.h"

#include "evita/v8_glue/gin_embedders.h"
#include "evita/v8_glue/v8.h"

namespace v8_glue {

PerIsolateData::PerIsolateData(v8::Isolate* isolate)
    : construct_mode_(kCreateNewObject),
      isolate_(isolate) {
  isolate_->SetData(gin::kEmbedderEvita, this);
}

PerIsolateData::~PerIsolateData() {
}

PerIsolateData* PerIsolateData::From(v8::Isolate* isolate) {
  return static_cast<PerIsolateData*>(isolate->GetData(gin::kEmbedderEvita));
}

}  // namespace v8_glue
