// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor_window.h"

#include "base/bind.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application.h"
#include "evita/vi_Frame.h"

namespace dom {

EditorWindow::EditorWindow() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
}

EditorWindow::~EditorWindow() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
}

v8_glue::WrapperInfo* EditorWindow::static_wrapper_info() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info,
      ("EditorWindow", Window::static_wrapper_info()));
  return &wrapper_info;
}

gin::ObjectTemplateBuilder EditorWindow::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  return GetEmptyObjectTemplateBuilder(isolate);
}

}  // namespace dom
