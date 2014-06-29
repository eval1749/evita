// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_delegate_h)
#define INCLUDE_evita_dom_view_delegate_h

#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/script_host_state.h"
#include "evita/dom/public/switch_value.h"
#include "evita/dom/windows/window_id.h"
#include "evita/dom/windows/window_id.h"

namespace domapi {
class FloatRect;
struct TabData;
class ViewEventHandler;

struct PopupWindowInit {
  dom::WindowId owner_id;
  int offset_x;
  int offset_y;
};

}  // namespace domapi

namespace text {
class Selection;
}

namespace dom {

class Document;
class EditorWindow;
class Form;
class FormWindow;
class TextWindow;
class Window;
using domapi::ScriptHostState;

struct TextWindowCompute {
  // Note: Value of |WindowCompute::Method| must match with JavaScript.
  enum class Method {
    EndOfWindow,
    EndOfWindowLine,
    MapPointToPositionShoultNotUse,
    HitTestTextPositionShouldNotUse,
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
  public: typedef domapi::Deferred<base::string16> GetFileNameForLoadResolver;
  public: typedef domapi::Deferred<base::string16> GetFileNameForSaveResolver;
  public: typedef domapi::Deferred<int> MessageBoxResolver;

  public: ViewDelegate() = default;
  public: virtual ~ViewDelegate() = default;

  public: virtual void AddWindow(WindowId parent_id, WindowId child_id) = 0;
  public: virtual void ChangeParentWindow(WindowId window_id,
                                          WindowId new_parent_window_id) = 0;
  public: virtual text::Posn ComputeOnTextWindow(
      WindowId window_id, const TextWindowCompute& data) = 0;
  public: virtual void CreateEditorWindow(const EditorWindow* window) = 0;

  // Create Form window
  public: virtual void CreateFormWindow(
      WindowId window_id, Form* form, const domapi::PopupWindowInit& init) = 0;

  public: virtual void CreateTableWindow(WindowId window_id,
                                         Document* document) = 0;
  public: virtual void CreateTextWindow(WindowId window_id,
                                        text::Selection* selection) = 0;
  public: virtual void DestroyWindow(WindowId window_id) = 0;

  // |DidStartScriptHost| is called when script host execute editor start
  // script.
  public: virtual void DidStartScriptHost(ScriptHostState state) = 0;

  // |DidHandleViewIdelEvent| is called when "idle" event handler returns.
  public: virtual void DidHandleViewIdelEvent(int hint) = 0;
  public: virtual void FocusWindow(WindowId window_id) = 0;

  // Get file name for load by using open file modal dialog box.
  public: virtual void GetFileNameForLoad(
      WindowId window_id, const base::string16& dir_path,
      const GetFileNameForLoadResolver& resolver) = 0;

  // Get file name for save by using open file modal dialog box.
  public: virtual void GetFileNameForSave(
      WindowId window_id, const base::string16& dir_path,
      const GetFileNameForSaveResolver& resolver) = 0;

  public: virtual base::string16 GetMetrics(const base::string16& name) = 0;

  // Get switch bool value
  public: virtual domapi::SwitchValue GetSwitch(
      const base::string16& name) = 0;

  // Get switch names
  public: virtual std::vector<base::string16> GetSwitchNames() = 0;

  public: virtual std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) = 0;
  public: virtual void HideWindow(WindowId window_id) = 0;

  // Get bounding rectangle of character at text offset.
  public: virtual domapi::FloatRect HitTestTextPosition(
      WindowId window_id, text::Posn offset) = 0;

  public: virtual void MakeSelectionVisible(WindowId window_id) = 0;
  public: virtual text::Posn MapPointToPosition(
      domapi::EventTargetId event_target_id, float x, float y) = 0;
  // Popup message box dialog box and return response code.
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      const MessageBoxResolver& callback) = 0;

  public: virtual void Reconvert(WindowId window_id,
                                 const base::string16& text) = 0;
  public: virtual void RealizeWindow(WindowId window_id) = 0;
  public: virtual void RegisterViewEventHandler(
      domapi::ViewEventHandler* event_handler) = 0;

  // Release capture from window or form control.
  public: virtual void ReleaseCapture(
      domapi::EventTargetId event_target_id) = 0;

  public: /* synchronous */ virtual void ScrollTextWindow(WindowId windowId,
                                                          int direction) = 0;
  // Release capture from window or form control.
  public: virtual void SetCapture(
      domapi::EventTargetId event_target_id) = 0;

  // Set text contents of status bar of specified top level window.
  public: virtual void SetStatusBar(WindowId window_id,
      const std::vector<base::string16>& texts) = 0;

  // Set switch value
  public: virtual void SetSwitch(const base::string16& name,
                                 const domapi::SwitchValue& new_value) = 0;
  
  public: virtual void SetTabData(WindowId window_id,
                                  const domapi::TabData& tab_data) = 0;
  // Set |TextWindow| zoom factor.
  public: virtual void SetTextWindowZoom(WindowId window_id, float zoom) = 0;

  public: virtual void ShowWindow(WindowId window_id) = 0;
  public: virtual void SplitHorizontally(WindowId left_window_id,
                                         WindowId new_right_window_id) = 0;
  public: virtual void SplitVertically(WindowId above_window_id,
                                       WindowId new_below_window_id) = 0;

  // Synchronous: Update window contents
  public: virtual void UpdateWindow(WindowId window_id) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_delegate_h)
