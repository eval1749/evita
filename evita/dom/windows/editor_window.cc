// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/editor_window.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "common/memory/singleton.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// EditorWindowList
//
class EditorWindowList : public common::Singleton<EditorWindowList> {
  friend class common::Singleton<EditorWindowList>;

  private: typedef std::vector<EditorWindow*> List;
  private: std::unordered_set<EditorWindow*> set_;

  private: EditorWindowList() = default;
  public: ~EditorWindowList() = default;

  private: List list() const {
    List list(set_.size());
    list.resize(0);
    for (auto window : set_) {
      list.push_back(window);
    }
    return std::move(list);
  }

  public: void Register(EditorWindow* window) {
    set_.insert(window);
  }
  public: void ResetForTesting() {
    set_.clear();
  }
  public: static List StaticList() {
    return instance()->list();
  }
  public: void Unregister(EditorWindow* window) {
    set_.erase(window);
  }

  DISALLOW_COPY_AND_ASSIGN(EditorWindowList);
};

//////////////////////////////////////////////////////////////////////
//
// EditorWindowWrapperInfo
//
class EditorWindowWrapperInfo :
    public v8_glue::DerivedWrapperInfo<EditorWindow, Window> {

  public: explicit EditorWindowWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~EditorWindowWrapperInfo() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &EditorWindowWrapperInfo::NewEditorWindow);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetProperty("list", &EditorWindowList::StaticList)
        .Build();
  }

  private: static EditorWindow* NewEditorWindow() {
    return new EditorWindow();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetMethod("setStatusBar", &EditorWindow::SetStatusBar);
  }

  DISALLOW_COPY_AND_ASSIGN(EditorWindowWrapperInfo);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditorWindow
//
DEFINE_SCRIPTABLE_OBJECT(EditorWindow, EditorWindowWrapperInfo);

EditorWindow::EditorWindow() {
  EditorWindowList::instance()->Register(this);
  ScriptController::instance()->view_delegate()->CreateEditorWindow(this);
}

EditorWindow::~EditorWindow() {
}

void EditorWindow::ResetForTesting() {
  EditorWindowList::instance()->ResetForTesting();
}

void EditorWindow::SetStatusBar(const std::vector<base::string16>& texts) {
  ScriptController::instance()->view_delegate()->SetStatusBar(
      window_id(), texts);
}

// Window
void EditorWindow::DidDestroyWindow() {
  Window::DidDestroyWindow();
  EditorWindowList::instance()->Unregister(this);
}

}  // namespace dom
