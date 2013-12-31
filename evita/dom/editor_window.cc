// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor_window.h"

#include "base/bind.h"
#include "evita/dom/script_thread.h"

namespace dom {

EditorWindow::EditorWindow() {
}

EditorWindow::~EditorWindow() {
}

v8_glue::WrapperInfo* EditorWindow::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info,
      ("EditorWindow", Window::static_wrapper_info()));
  return &wrapper_info;
}

gin::ObjectTemplateBuilder EditorWindow::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetObjectTemplateBuilderFromBase(isolate);
}

}  // namespace dom
