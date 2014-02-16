// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_view_delegate_impl_h)
#define INCLUDE_evita_view_delegate_impl_h

#include "evita/dom/view_delegate.h"

namespace dom {
class ViewEventHandler;
}

namespace views {

class ViewDelegateImpl : public dom::ViewDelegate {
  private: dom::ViewEventHandler* event_handler_;

  public: ViewDelegateImpl();
  public: virtual ~ViewDelegateImpl();

  public: dom::ViewEventHandler* event_handler() const {
    return event_handler_;
  }

  // ViewDelegate
  private: virtual void AddWindow(dom::WindowId parent_id,
                                  dom::WindowId child_id) override;
  private: virtual void ChangeParentWindow(dom::WindowId window_id,
      dom::WindowId new_parent_id) override;
  private: virtual void ComputeOnTextWindow(dom::WindowId window_id,
      dom::TextWindowCompute* data, base::WaitableEvent* event) override;
  private: virtual void CreateDialogBox(
      dom::DialogBoxId dialog_box_id) override;
  private: virtual void CreateEditorWindow(
      const dom::EditorWindow* window) override;
  private: virtual void CreateTableWindow(
      dom::WindowId window_id, dom::Document* document) override;
  private: virtual void CreateTextWindow(
      const dom::TextWindow* window) override;
  private: virtual void DestroyWindow(dom::WindowId window_id) override;
  private: virtual void FocusWindow(dom::WindowId window_id) override;
  private: virtual void GetFilenameForLoad(
      dom::WindowId window_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) override;
  private: virtual void GetFilenameForSave(
      dom::WindowId window_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) override;
  private: virtual void GetTableRowStates(dom::WindowId window_id,
      const std::vector<base::string16>& keys, int* states,
      base::WaitableEvent* event) override;
  private: virtual void LoadFile(dom::Document* document,
                                 const base::string16& filename,
                                 LoadFileCallback callback) override;
  private: virtual void MakeSelectionVisible(dom::WindowId window_id) override;
  private: virtual void MessageBox(dom::WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) override;
  public: virtual void Reconvert(dom::WindowId window_id, text::Posn start,
                                 text::Posn end) override;
  private: virtual void RealizeDialogBox(const dom::Form* form) override;
  private: virtual void RealizeWindow(dom::WindowId window_id) override;
  private: virtual void RegisterViewEventHandler(
      dom::ViewEventHandler* event_handler) override;
  private: virtual void ReleaseCapture(dom::WindowId window_id) override;
  private: virtual void SaveFile(dom::Document* document,
                                 const base::string16& filename,
                                 const SaveFileCallback& callback) override;
  private: virtual void SetCapture(dom::WindowId window_id) override;
  private: virtual void ShowDialogBox(
      dom::DialogBoxId dialog_box_id) override;
  private: virtual void SplitHorizontally(dom::WindowId left_window,
      dom::WindowId new_right_window) override;
  private: virtual void SplitVertically(dom::WindowId above_window,
      dom::WindowId new_below_window) override;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegateImpl);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_view_delegate_impl_h)
