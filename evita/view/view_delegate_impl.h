// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_view_delegate_impl_h)
#define INCLUDE_evita_view_delegate_impl_h

#include "evita/dom/view_delegate.h"

namespace dom {
class ViewEventHandler;
}

namespace view {

class ViewDelegateImpl : public dom::ViewDelegate {
  private: dom::ViewEventHandler* event_handler_;

  public: ViewDelegateImpl();
  public: virtual ~ViewDelegateImpl();

  public: dom::ViewEventHandler* event_handler() const {
    return event_handler_;
  }

  // ViewDelegate
  private: virtual void CreateEditorWindow(
      const dom::EditorWindow* window) override;
  private: virtual void CreateTextWindow(
      const dom::TextWindow* window) override;
  private: virtual void AddWindow(dom::WidgetId parent_id,
                                  dom::WidgetId child_id) override;
  private: virtual void DestroyWindow(dom::WidgetId widget_id) override;
  private: virtual void GetFilenameForLoad(
      dom::WidgetId widget_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) override;
  private: virtual void GetFilenameForSave(
      dom::WidgetId widget_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) override;
  private: virtual void LoadFile(dom::Document* document,
                                 const base::string16& filename) override;
  private: virtual void RealizeWindow(dom::WidgetId widget_id) override;
  private: virtual void RegisterViewEventHandler(
      dom::ViewEventHandler* event_handler) override;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegateImpl);
};

}  // namespace view

#endif //!defined(INCLUDE_evita_view_delegate_impl_h)
