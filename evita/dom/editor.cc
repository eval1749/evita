// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor.h"

#include "evita/cm_CmdProc.h"
#include "evita/gc/local.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_command.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {

// TODO(yosi): Until we enable |new EditorWindow()|, we use
// |editor.createFrame()|.
v8::Handle<v8::Object> CreateFrame() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  ASSERT_DOM_LOCKED();
  auto window = gc::MakeLocal(new EditorWindow());
  return window->GetWrapper(v8::Isolate::GetCurrent());
}

void SetKeyBinding(int key_code, v8::Handle<v8::Object> command) {
  ASSERT_DOM_LOCKED();
  if (!Command::g_pGlobalBinds) {
    // During DOM unit testing, |g_GlobalBinds| isn't initialized.
    return;
  }
  Command::g_pGlobalBinds->Bind(key_code, new ScriptCommand(command));
}

class EditorWrapperInfo : public v8_glue::WrapperInfo {
  public: EditorWrapperInfo() : v8_glue::WrapperInfo("Editor") {
  }
  public: ~EditorWrapperInfo() = default;

  public: virtual AbstractScriptable* singleton() const override {
    return Editor::instance();
  }

  public: virtual const char* singleton_name() const override {
    return "editor";
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
      .SetMethod("createFrame", CreateFrame)
      .SetMethod("setKeyBinding_", SetKeyBinding)
      .SetProperty("version", &Editor::version);
  }
};

}  // namespace

Editor::Editor() {
}

v8_glue::WrapperInfo* Editor::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EditorWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

const base::string16& Editor::version() const {
  return Application::instance()->version();
}

}  // namespace dom
