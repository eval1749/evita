// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/editor_window.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "common/memory/singleton.h"
#include "evita/dom/script_host.h"
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

  public: typedef std::vector<EditorWindow*> List;
  private: std::unordered_set<EditorWindow*> set_;

  private: EditorWindowList() = default;
  public: ~EditorWindowList() = default;

  public: List list() const {
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
  public: void Unregister(EditorWindow* window) {
    set_.erase(window);
  }

  DISALLOW_COPY_AND_ASSIGN(EditorWindowList);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditorWindow
//
EditorWindow::EditorWindow() {
  EditorWindowList::instance()->Register(this);
  ScriptHost::instance()->view_delegate()->CreateEditorWindow(this);
}

EditorWindow::~EditorWindow() {
}

// static
EditorWindowList::List EditorWindow::list() {
  return EditorWindowList::instance()->list();
}

// static
EditorWindow* EditorWindow::NewEditorWindow() {
  return new EditorWindow();
}

void EditorWindow::ResetForTesting() {
  EditorWindowList::instance()->ResetForTesting();
}

void EditorWindow::SetStatusBar(const std::vector<base::string16>& texts) {
  ScriptHost::instance()->view_delegate()->SetStatusBar(
      window_id(), texts);
}

// Window
void EditorWindow::DidDestroyWindow() {
  Window::DidDestroyWindow();
  EditorWindowList::instance()->Unregister(this);
}

}  // namespace dom
