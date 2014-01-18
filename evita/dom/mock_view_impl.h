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
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl : public dom::ViewDelegate {
  public: MockViewImpl();
  public: virtual ~MockViewImpl();

  MOCK_METHOD2(AddWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(WindowId, WindowId));
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD2(CreateTableWindow, void(WindowId, Document*));
  MOCK_METHOD1(CreateTextWindow, void(const TextWindow*));
  MOCK_METHOD1(DestroyWindow, void(WindowId));
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
  MOCK_METHOD2(LoadFile, void(Document*, const base::string16&));
  MOCK_METHOD1(MakeSelectionVisible, void(WindowId));
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) override;
  MOCK_METHOD2(SaveFile, void(Document*, const base::string16&));
  MOCK_METHOD1(RealizeWindow, void(WindowId));
  MOCK_METHOD1(RegisterViewEventHandler, void(ViewEventHandler*));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
