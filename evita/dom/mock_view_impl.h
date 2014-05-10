// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_mock_view_impl_h)
#define INCLUDE_evita_dom_mock_view_impl_h

#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4365 4628)
#include "gmock/gmock.h"
#pragma warning(pop)
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl : public dom::ViewDelegate {
  private: bool check_spelling_result_;
  private: std::vector<base::string16> spelling_suggestions_;

  public: MockViewImpl();
  public: virtual ~MockViewImpl();

  // ViewDelegate
  MOCK_METHOD2(AddWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ComputeOnTextWindow,
      text::Posn(WindowId, const TextWindowCompute&));
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD3(CreateFormWindow, void(WindowId, Form*,
                                      const domapi::PopupWindowInit&));
  MOCK_METHOD2(CreateTableWindow, void(WindowId, Document*));
  MOCK_METHOD1(CreateTextWindow, void(const TextWindow*));
  MOCK_METHOD1(DestroyWindow, void(WindowId));
  MOCK_METHOD1(DidStartScriptHost, void(ScriptHostState));
  MOCK_METHOD1(DidHandleViewIdelEvent, void(int));
  MOCK_METHOD1(FocusWindow, void(WindowId));
  public: virtual void GetFileNameForLoad(WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForLoadResolver& resolver) override;
  public: virtual void GetFileNameForSave(WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForSaveResolver& resolver) override;
  MOCK_METHOD1(GetMetrics, base::string16(const base::string16&));
  MOCK_METHOD1(GetSwitch, domapi::SwitchValue(const base::string16& name));
  MOCK_METHOD0(GetSwitchNames, std::vector<base::string16>());
  public: std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) override;
  MOCK_METHOD1(HideWindow, void(WindowId));
  MOCK_METHOD2(HitTestTextPosition,
               domapi::FloatRect(WindowId, text::Posn));
  MOCK_METHOD1(MakeSelectionVisible, void(WindowId));
  MOCK_METHOD3(MapPointToPosition,
      text::Posn(domapi::EventTargetId, float x, float y));
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      const MessageBoxResolver& resolver) override;
  MOCK_METHOD2(Reconvert, void(WindowId, const base::string16&));
  MOCK_METHOD1(RealizeWindow, void(WindowId));
  MOCK_METHOD1(RegisterViewEventHandler, void(domapi::ViewEventHandler*));
  MOCK_METHOD1(ReleaseCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(ScrollTextWindow, void(WindowId, int));
  MOCK_METHOD1(SetCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(SetStatusBar,
      void(WindowId, const std::vector<base::string16>&));
  MOCK_METHOD2(SetSwitch, void(const base::string16&,
                               const domapi::SwitchValue&));
  MOCK_METHOD2(SetTabData, void(WindowId, const domapi::TabData&));
  MOCK_METHOD2(SetTextWindowZoom, void(WindowId, float));
  MOCK_METHOD1(ShowWindow, void(WindowId));
  MOCK_METHOD2(SplitHorizontally, void(WindowId, WindowId));
  MOCK_METHOD2(SplitVertically, void(WindowId, WindowId));
  MOCK_METHOD1(UpdateWindow, void(WindowId));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
