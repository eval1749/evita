// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ginx/function_template_builder.h"

namespace ginx {

FunctionTemplateBuilder::FunctionTemplateBuilder(
    v8::Isolate* isolate,
    v8::Local<v8::FunctionTemplate> function_template)
    : isolate_(isolate), template_(function_template) {}

FunctionTemplateBuilder::FunctionTemplateBuilder(v8::Isolate* isolate)
    : FunctionTemplateBuilder(isolate, v8::FunctionTemplate::New(isolate)) {}

FunctionTemplateBuilder::~FunctionTemplateBuilder() {}

FunctionTemplateBuilder& FunctionTemplateBuilder::SetImpl(
    const base::StringPiece& name,
    v8::Local<v8::Data> val) {
  template_->Set(gin::StringToSymbol(isolate_, name), val);
  return *this;
}

FunctionTemplateBuilder& FunctionTemplateBuilder::SetPropertyImpl(
    const base::StringPiece& name,
    v8::Local<v8::FunctionTemplate> getter,
    v8::Local<v8::FunctionTemplate> setter) {
  template_->SetAccessorProperty(gin::StringToSymbol(isolate_, name), getter,
                                 setter);
  return *this;
}

v8::Local<v8::FunctionTemplate> FunctionTemplateBuilder::Build() {
  v8::Local<v8::FunctionTemplate> result = template_;
  template_.Clear();
  return result;
}

}  // namespace ginx
