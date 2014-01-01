// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor_window.h"

#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/constructor_template.h"


namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// EditorWindowWrapperInfo
//
class EditorWindowWrapperInfo : public v8_glue::WrapperInfo {
  public: EditorWindowWrapperInfo() : v8_glue::WrapperInfo("EditorWindow") {
  }
  public: ~EditorWindowWrapperInfo() = default;

  private: virtual WrapperInfo* inherit_from() const override {
    return Window::static_wrapper_info();
  }

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &EditorWindowWrapperInfo::NewEditorWindow);
  }

  private: static EditorWindow* NewEditorWindow() {
    return new EditorWindow();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    v8_glue::WrapperInfo::SetupInstanceTemplate(builder);
    // TODO(yosi) Add EditorWindow properties.
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditorWindow
//
EditorWindow::EditorWindow() {
  ScriptController::instance()->view_delegate()->CreateEditorWindow(this);
}

EditorWindow::~EditorWindow() {
}

v8_glue::WrapperInfo* EditorWindow::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EditorWindowWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
