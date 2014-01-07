// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_delegate_h)
#define INCLUDE_evita_dom_view_delegate_h

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/widget_id.h"

namespace dom {

class EditorWindow;
class TextWindow;
class ViewEventHandler;
class Window;

class ViewDelegate {
  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForSaveCallback;

  public: ViewDelegate() = default;
  public: virtual ~ViewDelegate() = default;

  public: virtual void CreateEditorWindow(const EditorWindow* window) = 0;
  public: virtual void CreateTextWindow(const TextWindow* window) = 0;
  public: virtual void AddWindow(WidgetId parent_id, WidgetId child_id) = 0;
  public: virtual void DestroyWindow(WidgetId widget_id) = 0;
  public: virtual void GetFilenameForSave(
      WidgetId widget_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) = 0;
  public: virtual void RealizeWindow(WidgetId widget_id) = 0;
  public: virtual void RegisterViewEventHandler(
      ViewEventHandler* event_handler) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_delegate_h)
