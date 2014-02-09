// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_window_h)
#define INCLUDE_evita_dom_editor_window_h

#include "evita/dom/window.h"

namespace dom {

// |dom::EditoWindow| corresponds to |editor::EditorWidget|, also known as
// |Frame|.
class EditorWindow : public v8_glue::Scriptable<EditorWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(EditorWindow);

  public: EditorWindow();
  public: virtual ~EditorWindow();

  public: static void ResetForTesting();

  // Window
  private: virtual void DidDestroyWindow() override;
  private: virtual void DidRealizeWindow() override;

  DISALLOW_COPY_AND_ASSIGN(EditorWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_editor_window_h)
