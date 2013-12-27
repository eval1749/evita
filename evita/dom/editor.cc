// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor.h"

#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"

namespace dom {

v8_glue::ScriptWrapperInfo* Editor::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::ScriptWrapperInfo, wrapper_info, ("Editor"));
  return &wrapper_info;
}

const base::string16& Editor::version() const {
  return Application::instance().version();
}

gin::ObjectTemplateBuilder Editor::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetEmptyObjectTemplateBuilder(isolate)
      .SetProperty("version", &Editor::version);
}

}  // namespace dom
