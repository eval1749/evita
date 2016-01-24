// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/editor_window.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "common/memory/singleton.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// EditorWindowList
//
class EditorWindowList final : public common::Singleton<EditorWindowList> {
 public:
  using List = std::vector<EditorWindow*>;

  ~EditorWindowList() final = default;

  List list() const {
    List list(set_.size());
    list.resize(0);
    for (auto window : set_) {
      list.push_back(window);
    }
    return std::move(list);
  }

  void Register(EditorWindow* window) { set_.insert(window); }
  void ResetForTesting() { set_.clear(); }
  void Unregister(EditorWindow* window) { set_.erase(window); }

 private:
  friend class common::Singleton<EditorWindowList>;

  EditorWindowList() = default;

  std::unordered_set<EditorWindow*> set_;

  DISALLOW_COPY_AND_ASSIGN(EditorWindowList);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditorWindow
//
EditorWindow::EditorWindow(ScriptHost* script_host) : Scriptable(script_host) {
  EditorWindowList::instance()->Register(this);
  script_host->view_delegate()->CreateEditorWindow(window_id());
}

EditorWindow::~EditorWindow() {}

// static
EditorWindowList::List EditorWindow::list() {
  return EditorWindowList::instance()->list();
}

void EditorWindow::ResetForTesting() {
  EditorWindowList::instance()->ResetForTesting();
}

void EditorWindow::SetStatusBar(const std::vector<base::string16>& texts) {
  script_host()->view_delegate()->SetStatusBar(window_id(), texts);
}

// Window
void EditorWindow::DidDestroyWindow() {
  Window::DidDestroyWindow();
  EditorWindowList::instance()->Unregister(this);
}

}  // namespace dom
