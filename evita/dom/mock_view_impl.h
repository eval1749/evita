// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_MOCK_VIEW_IMPL_H_
#define EVITA_DOM_MOCK_VIEW_IMPL_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/view_delegate.h"
#include "gmock/gmock.h"

namespace dom {

class MockViewImpl final : public domapi::ViewDelegate {
 public:
  MockViewImpl();
  ~MockViewImpl() final;

  // ViewDelegate
  MOCK_METHOD2(AddWindow, void(domapi::WindowId, domapi::WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(domapi::WindowId, domapi::WindowId));
  MOCK_METHOD2(ComputeOnTextWindow,
               text::Offset(domapi::WindowId,
                            const domapi::TextWindowCompute&));
  MOCK_METHOD1(CreateEditorWindow, void(domapi::WindowId));
  MOCK_METHOD3(CreateFormWindow,
               void(domapi::WindowId, Form*, const domapi::PopupWindowInit&));
  MOCK_METHOD2(CreateTableWindow, void(domapi::WindowId, TextDocument*));
  MOCK_METHOD2(CreateTextWindow, void(domapi::WindowId, text::Selection*));
  MOCK_METHOD1(CreateVisualWindow, void(domapi::WindowId window_id));
  MOCK_METHOD1(DestroyWindow, void(domapi::WindowId));
  MOCK_METHOD1(DidStartScriptHost, void(domapi::ScriptHostState));
  MOCK_METHOD0(DidUpdateDom, void());
  MOCK_METHOD1(FocusWindow, void(domapi::WindowId));
  void GetFileNameForLoad(domapi::WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForLoadResolver& resolver) final;
  void GetFileNameForSave(domapi::WindowId window_id,
                          const base::string16& dir_path,
                          const GetFileNameForSaveResolver& resolver) final;
  MOCK_METHOD2(GetMetrics,
               void(const base::string16&, const domapi::StringPromise&));
  MOCK_METHOD1(GetSwitch, domapi::SwitchValue(const base::string16& name));
  MOCK_METHOD0(GetSwitchNames, std::vector<base::string16>());
  std::vector<int> GetTableRowStates(
      domapi::WindowId window_id,
      const std::vector<base::string16>& keys) final;
  MOCK_METHOD1(HideWindow, void(domapi::WindowId));
  MOCK_METHOD2(HitTestTextPosition,
               domapi::FloatRect(domapi::WindowId, text::Offset));
  MOCK_METHOD1(MakeSelectionVisible, void(domapi::WindowId));
  MOCK_METHOD4(MapTextFieldPointToOffset,
               void(domapi::EventTargetId,
                    float x,
                    float y,
                    const domapi::IntegerPromise& promise));
  MOCK_METHOD4(MapTextWindowPointToOffset,
               void(domapi::EventTargetId,
                    float x,
                    float y,
                    const domapi::IntegerPromise& promise));
  void MessageBox(domapi::WindowId window_id,
                  const base::string16& message,
                  const base::string16& title,
                  int flags,
                  const MessageBoxResolver& resolver) final;
  void PaintVisualDocument(
      domapi::WindowId window_id,
      std::unique_ptr<visuals::DisplayItemList> display_item_list) final;
  MOCK_METHOD2(Reconvert, void(domapi::WindowId, const base::string16&));
  MOCK_METHOD1(RealizeWindow, void(domapi::WindowId));
  MOCK_METHOD1(ReleaseCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(ScrollTextWindow, void(domapi::WindowId, int));
  MOCK_METHOD1(SetCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(SetStatusBar,
               void(domapi::WindowId, const std::vector<base::string16>&));
  MOCK_METHOD2(SetSwitch,
               void(const base::string16&, const domapi::SwitchValue&));
  MOCK_METHOD2(SetTabData, void(domapi::WindowId, const domapi::TabData&));
  MOCK_METHOD2(SetTextWindowZoom, void(domapi::WindowId, float));
  MOCK_METHOD1(ShowWindow, void(domapi::WindowId));
  MOCK_METHOD2(SplitHorizontally, void(domapi::WindowId, domapi::WindowId));
  MOCK_METHOD2(SplitVertically, void(domapi::WindowId, domapi::WindowId));

  MOCK_METHOD1(StartTraceLog, void(const std::string& config));
  MOCK_METHOD1(StopTraceLog,
               void(const domapi::TraceLogOutputCallback& callback));

  MOCK_METHOD1(UpdateWindow, void(domapi::WindowId));

 private:
  bool check_spelling_result_;
  std::vector<base::string16> spelling_suggestions_;

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif  // EVITA_DOM_MOCK_VIEW_IMPL_H_
