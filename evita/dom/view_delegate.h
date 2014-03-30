// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_delegate_h)
#define INCLUDE_evita_dom_view_delegate_h

#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/public/dialog_box_id.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/script_host_state.h"
#include "evita/dom/windows/window_id.h"
#include "evita/dom/windows/window_id.h"

namespace domapi {
class FloatPoint;
struct LoadFileCallbackData;
struct SaveFileCallbackData;
struct TabData;
class ViewEventHandler;
}

namespace dom {

class Buffer;
class Document;
class EditorWindow;
class Form;
class TextWindow;
class Window;
using domapi::ScriptHostState;

struct TextWindowCompute {
  // Note: Value of |WindowCompute::Method| must match with JavaScript.
  enum class Method {
    EndOfWindow,
    EndOfWindowLine,
    MapPointToPositionShoultNotUse,
    MapPositionToPointShouldNotUse,
    MoveScreen,
    MoveWindow,
    MoveWindowLine,
    StartOfWindow,
    StartOfWindowLine,
  };
  Method method;
  text::Posn position;
  int count;
  float x;
  float y;
};

class ViewDelegate {
  public: typedef domapi::Deferred<bool> CheckSpellingDeferred;

  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForLoadCallback;

  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForSaveCallback;

  public: typedef domapi::Deferred<std::vector<base::string16>>
      GetSpellingSuggestionsDeferred;

  public: typedef base::Callback<void(const domapi::LoadFileCallbackData&)>
      LoadFileCallback;

  public: typedef base::Callback<void(int response_code)>
      MessageBoxCallback;

  public: typedef base::Callback<void(const domapi::SaveFileCallbackData&)>
      SaveFileCallback;

  public: ViewDelegate() = default;
  public: virtual ~ViewDelegate() = default;

  public: virtual void AddWindow(WindowId parent_id, WindowId child_id) = 0;
  public: virtual void ChangeParentWindow(WindowId window_id,
                                          WindowId new_parent_window_id) = 0;
  public: virtual void CheckSpelling(const base::string16& word_to_check,
        const CheckSpellingDeferred& callback) = 0;
  public: virtual text::Posn ComputeOnTextWindow(
      WindowId window_id, const TextWindowCompute& data) = 0;
  public: virtual void CreateEditorWindow(const EditorWindow* window) = 0;
  public: virtual void CreateFindDialogBox(Form* form) = 0;
  public: virtual void CreateFormWindow(WindowId, const Form* form) = 0;
  public: virtual void CreateTableWindow(WindowId window_id,
                                         Document* document) = 0;
  public: virtual void CreateTextWindow(const TextWindow* window) = 0;
  public: virtual void DestroyWindow(WindowId window_id) = 0;
  public: virtual void DidChangeFormContents(
      domapi::DialogBoxId dialog_box_id) = 0;

  // |DidStartScriptHost| is called when script host execute editor start
  // script.
  public: virtual void DidStartScriptHost(ScriptHostState state) = 0;

  // |DidHandleViewIdelEvent| is called when "idle" event handler returns.
  public: virtual void DidHandleViewIdelEvent(int hint) = 0;
  public: virtual void FocusWindow(WindowId window_id) = 0;
  public: virtual void GetFilenameForLoad(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) = 0;
  public: virtual void GetFilenameForSave(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) = 0;
  public: virtual base::string16 GetMetrics(const base::string16& name) = 0;
  public: virtual void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsDeferred& callback) = 0;
  public: virtual std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) = 0;
  public: virtual void HideWindow(WindowId window_id) = 0;
  public: virtual void LoadFile(Document* document,
                                const base::string16& filename,
                                LoadFileCallback callback) = 0;
  public: virtual void MakeSelectionVisible(WindowId window_id) = 0;
  public: virtual text::Posn MapPointToPosition(WindowId window_id,
                                                float x, float y) = 0;
  public: virtual domapi::FloatPoint MapPositionToPoint(
      WindowId window_id, text::Posn offset) = 0;
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) = 0;
  public: virtual void Reconvert(WindowId window_id, text::Posn start,
                                 text::Posn end) = 0;
  public: virtual void RealizeDialogBox(domapi::DialogBoxId dialog_box_id) = 0;
  public: virtual void RealizeWindow(WindowId window_id) = 0;
  public: virtual void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) = 0;
  public: virtual void ReleaseCapture(WindowId window_id) = 0;
  public: virtual void SaveFile(Document* document,
                                const base::string16& filename,
                                const SaveFileCallback& callback) = 0;
  public: /* synchronous */ virtual void ScrollTextWindow(WindowId windowId,
                                                          int direction) = 0;
  public: virtual void SetCapture(WindowId window_id) = 0;
  // Set text contents of status bar of specified top level window.
  public: virtual void SetStatusBar(WindowId window_id,
      const std::vector<base::string16>& texts) = 0;
  public: virtual void SetTabData(WindowId window_id,
                                  const domapi::TabData& tab_data) = 0;
  public: virtual void ShowDialogBox(domapi::DialogBoxId dialog_box_id) = 0;
  public: virtual void ShowWindow(WindowId window_id) = 0;
  public: virtual void SplitHorizontally(WindowId left_window_id,
                                         WindowId new_right_window_id) = 0;
  public: virtual void SplitVertically(WindowId above_window_id,
                                       WindowId new_below_window_id) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_delegate_h)
