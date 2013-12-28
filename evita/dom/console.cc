// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/console.h"

#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"
#include "evita/vi_Buffer.h"

namespace dom {

namespace {
// TODO(yosi): Move InternBuffer to Application
Buffer* InternBuffer(const base::string16& name) {
  if (auto const buffer = Application::instance()->FindBuffer(name.c_str()))
    return buffer;
  return Application::instance()->NewBuffer(name.c_str());
}


void Log(const base::vector<std::string16>& messages) {
  auto const buffer = InternBuffer(L"*console log*");
  for (auto message : messages) {
    buffer->Insert(buffer->GetEnd(), message.c_str());
  }
  buffer->Insert(buffer->GetEnd(), L"\n");
}

}  // namespace

v8_glue::WrapperInfo* Console::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info, ("Console"));
  return &wrapper_info;
}

gin::ObjectTemplateBuilder Console::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetEmptyObjectTemplateBuilder(isolate)
    .SetMethod("log", Log);
}

}  // namespace dom
