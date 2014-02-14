// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_mock_view_impl_h)
#define INCLUDE_evita_dom_mock_view_impl_h

#include "base/basictypes.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4365 4628)
#include "gmock/gmock.h"
#pragma warning(pop)
#include "evita/dom/public/api_callback.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl : public dom::ViewDelegate {
  private: domapi::LoadFileCallbackData load_file_callback_data_;
  private: domapi::SaveFileCallbackData save_file_callback_data_;

  public: MockViewImpl();
  public: virtual ~MockViewImpl();

  public: void SetLoadFileCallbackData(
      const domapi::LoadFileCallbackData& data);
  public: void SetSaveFileCallbackData(
      const domapi::SaveFileCallbackData& data);

  // ViewDelegate
  MOCK_METHOD2(AddWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(WindowId, WindowId));
  MOCK_METHOD3(ComputeOnTextWindow, void(WindowId,
                                       TextWindowCompute*,
                                       base::WaitableEvent*));
  MOCK_METHOD1(CreateDialogBox, void(DialogBoxId));
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD2(CreateTableWindow, void(WindowId, Document*));
  MOCK_METHOD1(CreateTextWindow, void(const TextWindow*));
  MOCK_METHOD1(DestroyWindow, void(WindowId));
  MOCK_METHOD2(DoFind, void(DialogBoxId, text::Direction));
  MOCK_METHOD1(FocusWindow, void(WindowId));
  public: virtual void GetFilenameForLoad(WindowId window_id,
    const base::string16& dir_path,
    GetFilenameForLoadCallback callback) override;
  public: virtual void GetFilenameForSave(WindowId window_id,
    const base::string16& dir_path,
    GetFilenameForSaveCallback callback) override;
  public: void GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys, int* states,
      base::WaitableEvent* event) override;
  public: void LoadFile(Document* document, const base::string16& filename,
                        LoadFileCallback callback);
  MOCK_METHOD1(MakeSelectionVisible, void(WindowId));
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) override;
  MOCK_METHOD3(Reconvert, void(WindowId, text::Posn, text::Posn));
  public: virtual void SaveFile(Document* document,
                                const base::string16& filename,
                                const SaveFileCallback& callback) override;
  MOCK_METHOD1(RealizeDialogBox, void(const Form*));
  MOCK_METHOD1(RealizeWindow, void(WindowId));
  MOCK_METHOD1(RegisterViewEventHandler, void(ViewEventHandler*));
  MOCK_METHOD1(ReleaseCapture, void(WindowId));
  MOCK_METHOD1(SetCapture, void(WindowId));
  MOCK_METHOD1(ShowDialogBox, void(DialogBoxId));
  MOCK_METHOD2(SplitHorizontally, void(WindowId, WindowId));
  MOCK_METHOD2(SplitVertically, void(WindowId, WindowId));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
