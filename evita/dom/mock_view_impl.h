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
#include "evita/dom/public/api_callback.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl : public dom::ViewDelegate {
  private: bool check_spelling_result_;
  private: std::vector<base::string16> spelling_suggestions_;
  private: domapi::LoadFileCallbackData load_file_callback_data_;
  private: domapi::SaveFileCallbackData save_file_callback_data_;

  public: MockViewImpl();
  public: virtual ~MockViewImpl();

  public: void set_check_spelling_result(bool result) {
    check_spelling_result_ = result;
  }
  public: void set_spelling_suggestions(
      const std::vector<base::string16>& spelling_suggestions) {
    spelling_suggestions_ = spelling_suggestions;
  }
  public: void SetLoadFileCallbackData(
      const domapi::LoadFileCallbackData& data);
  public: void SetSaveFileCallbackData(
      const domapi::SaveFileCallbackData& data);

  // ViewDelegate
  MOCK_METHOD2(AddWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(WindowId, WindowId));
  public: virtual void CheckSpelling(const base::string16& word_to_check,
      const CheckSpellingDeferred& deferred) override;
  MOCK_METHOD2(ComputeOnTextWindow,
      text::Posn(WindowId, const TextWindowCompute&));
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD2(CreateFormWindow, void(WindowId, const Form*));
  MOCK_METHOD2(CreateTableWindow, void(WindowId, Document*));
  MOCK_METHOD1(CreateTextWindow, void(const TextWindow*));
  MOCK_METHOD1(DestroyWindow, void(WindowId));
  MOCK_METHOD1(DidStartScriptHost, void(ScriptHostState));
  MOCK_METHOD1(DidHandleViewIdelEvent, void(int));
  MOCK_METHOD1(FocusWindow, void(WindowId));
  public: virtual void GetFilenameForLoad(WindowId window_id,
    const base::string16& dir_path,
    GetFilenameForLoadCallback callback) override;
  public: virtual void GetFilenameForSave(WindowId window_id,
    const base::string16& dir_path,
    GetFilenameForSaveCallback callback) override;
  MOCK_METHOD1(GetMetrics, base::string16(const base::string16&));
  public: virtual void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsDeferred& deferred) override;
  public: std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) override;
  MOCK_METHOD1(HideWindow, void(WindowId));
  public: void LoadFile(Document* document, const base::string16& filename,
                        LoadFileCallback callback);
  MOCK_METHOD1(MakeSelectionVisible, void(WindowId));
  MOCK_METHOD3(MapPointToPosition, text::Posn(WindowId, float x, float y));
  MOCK_METHOD2(MapPositionToPoint,
               domapi::FloatPoint(WindowId, text::Posn));
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) override;
  MOCK_METHOD3(Reconvert, void(WindowId, text::Posn, text::Posn));
  public: virtual void SaveFile(Document* document,
                                const base::string16& filename,
                                const SaveFileCallback& callback) override;
  MOCK_METHOD1(RealizeWindow, void(WindowId));
  MOCK_METHOD1(RegisterViewEventHandler, void(domapi::ViewEventHandler*));
  MOCK_METHOD1(ReleaseCapture, void(WindowId));
  MOCK_METHOD2(ScrollTextWindow, void(WindowId, int));
  MOCK_METHOD1(SetCapture, void(WindowId));
  MOCK_METHOD2(SetStatusBar,
      void(WindowId, const std::vector<base::string16>&));
  MOCK_METHOD2(SetTabData, void(WindowId, const domapi::TabData&));
  MOCK_METHOD1(ShowWindow, void(WindowId));
  MOCK_METHOD2(SplitHorizontally, void(WindowId, WindowId));
  MOCK_METHOD2(SplitVertically, void(WindowId, WindowId));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
