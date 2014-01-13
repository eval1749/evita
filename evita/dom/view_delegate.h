// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_delegate_h)
#define INCLUDE_evita_dom_view_delegate_h

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/window_id.h"

namespace dom {

class Document;
class EditorWindow;
class TextWindow;
class ViewEventHandler;
class Window;

class ViewDelegate {
  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForLoadCallback;

  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForSaveCallback;

  public: typedef base::Callback<void(int response_code)>
      MessageBoxCallback;

  public: ViewDelegate() = default;
  public: virtual ~ViewDelegate() = default;

  public: virtual void ChangeParentWindow(WindowId window_id,
                                          WindowId new_parent_window_id) = 0;
  public: virtual void CreateEditorWindow(const EditorWindow* window) = 0;
  public: virtual void CreateTextWindow(const TextWindow* window) = 0;
  public: virtual void AddWindow(WindowId parent_id, WindowId child_id) = 0;
  public: virtual void DestroyWindow(WindowId window_id) = 0;
  public: virtual void FocusWindow(WindowId window_id) = 0;
  public: virtual void GetFilenameForLoad(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) = 0;
  public: virtual void GetFilenameForSave(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) = 0;
  public: virtual void LoadFile(Document* document,
                                const base::string16& filename) = 0;

  public: virtual void MakeSelectionVisible(WindowId window_id) = 0;
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) = 0;
  public: virtual void RealizeWindow(WindowId window_id) = 0;
  public: virtual void RegisterViewEventHandler(
      ViewEventHandler* event_handler) = 0;
  public: virtual void SaveFile(Document* document,
                                const base::string16& filename) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_delegate_h)
