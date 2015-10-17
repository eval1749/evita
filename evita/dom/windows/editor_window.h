// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_EDITOR_WINDOW_H_
#define EVITA_DOM_WINDOWS_EDITOR_WINDOW_H_

#include <vector>

#include "evita/dom/windows/window.h"

namespace dom {

namespace bindings {
class EditorWindowClass;
}

// |dom::EditoWindow| corresponds to |editor::EditorWidget|, also known as
// |Frame|.
class EditorWindow final : public v8_glue::Scriptable<EditorWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(EditorWindow);

 public:
  // TODO(eval1749): We should not allow to create |EditorWindow| from C++.
  EditorWindow();
  ~EditorWindow() final;

  static void ResetForTesting();

 private:
  friend class bindings::EditorWindowClass;

  static std::vector<EditorWindow*> list();

  // Set status bar texts.
  void EditorWindow::SetStatusBar(const std::vector<base::string16>& texts);

  // Window
  void DidDestroyWindow() final;

  DISALLOW_COPY_AND_ASSIGN(EditorWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_EDITOR_WINDOW_H_
