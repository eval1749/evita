// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_mock_view_impl_h)
#define INCLUDE_evita_dom_mock_view_impl_h

#include "base/basictypes.h"
#include "base/callback.h"
#include "gmock/gmock.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl : public dom::ViewDelegate {
  public: MockViewImpl();
  public: virtual ~MockViewImpl();
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD1(CreateTextWindow, void(const TextWindow*));
  MOCK_METHOD2(AddWindow, void(WidgetId, WidgetId));
  MOCK_METHOD1(DestroyWindow, void(WidgetId));
  MOCK_METHOD3(GetSaveFilename, void(WidgetId, const base::string16&,
                                     GetSaveFilenameCallback));
  MOCK_METHOD1(RealizeWindow, void(WidgetId));
  MOCK_METHOD1(RegisterViewEventHandler, void(ViewEventHandler*));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
