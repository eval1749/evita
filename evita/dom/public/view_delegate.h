// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_VIEW_DELEGATE_H_
#define EVITA_DOM_PUBLIC_VIEW_DELEGATE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/geometry.h"
#include "evita/dom/public/promise.h"
#include "evita/dom/public/script_host_state.h"
#include "evita/dom/public/switch_value.h"
#include "evita/dom/public/window_id.h"
#include "evita/text/offset.h"

// TODO(eval1749): We should not use DOM object in DOM API.
namespace dom {
class TextDocument;
}

namespace visuals {
class DisplayItemList;
}

namespace domapi {

class Form;
struct TabData;
class TextAreaDisplayItem;

using TraceLogOutputCallback =
    base::Callback<void(const std::string& chunk, bool has_more_events)>;

struct TextWindowCompute final {
  // Note: Value of |WindowCompute::Method| must match with JavaScript.
  enum class Method {
    EndOfWindow,
    EndOfWindowLine,
    HitTestPointShoultNotUse,
    HitTestTextPositionShouldNotUse,
    MoveScreen,
    MoveWindow,
    MoveWindowLine,
    StartOfWindow,
    StartOfWindowLine,
  };
  Method method;
  text::Offset position;
  int count;
  float x;
  float y;
};

//////////////////////////////////////////////////////////////////////
//
// ViewDelegate
//
class ViewDelegate {
 public:
  using GetFileNameForLoadResolver = Promise<base::string16>;
  using GetFileNameForSaveResolver = Promise<base::string16>;
  using MessageBoxResolver = Promise<int>;

  virtual ~ViewDelegate();

  virtual void AddWindow(WindowId parent_id, WindowId child_id) = 0;
  virtual void ChangeParentWindow(WindowId window_id,
                                  WindowId new_parent_window_id) = 0;
  virtual void CreateEditorWindow(domapi::WindowId window_id) = 0;

  // Create Form window
  virtual void CreateFormWindow(WindowId window_id,
                                WindowId owner_window_id,
                                const IntRect& bounds,
                                const base::string16& title) = 0;

  virtual void CreateTextWindow(WindowId window_id) = 0;

  // Create a window for displaying visual document.
  virtual void CreateVisualWindow(WindowId window_id) = 0;

  virtual void DestroyWindow(WindowId window_id) = 0;

  // |DidUpdateDom()| is called when DOM thread finishes DOM
  // modification.
  virtual void DidUpdateDom() = 0;

  // |DidStartScriptHost| is called when script host execute editor start
  // script.
  virtual void DidStartScriptHost(ScriptHostState state) = 0;

  virtual void FocusWindow(WindowId window_id) = 0;

  // Get file name for load by using open file modal dialog box.
  virtual void GetFileNameForLoad(
      WindowId window_id,
      const base::string16& dir_path,
      const GetFileNameForLoadResolver& resolver) = 0;

  // Get file name for save by using open file modal dialog box.
  virtual void GetFileNameForSave(
      WindowId window_id,
      const base::string16& dir_path,
      const GetFileNameForSaveResolver& resolver) = 0;

  virtual void GetMetrics(const base::string16& name,
                          const StringPromise& promise) = 0;

  // Get switch bool value
  virtual SwitchValue GetSwitch(const base::string16& name) = 0;

  // Get switch names
  virtual std::vector<base::string16> GetSwitchNames() = 0;

  virtual void HideWindow(WindowId window_id) = 0;

// Popup message box dialog box and return response code.
#undef MessageBox
  virtual void MessageBox(WindowId window_id,
                          const base::string16& message,
                          const base::string16& title,
                          int flags,
                          const MessageBoxResolver& callback) = 0;

  virtual void PaintForm(WindowId window_id, std::unique_ptr<Form> form) = 0;

  virtual void PaintTextArea(
      WindowId window_id,
      std::unique_ptr<TextAreaDisplayItem> display_item) = 0;

  // TODO(eval1749): We should move |PaintVisualDocument()| to paint delegate.
  virtual void PaintVisualDocument(
      WindowId window_id,
      std::unique_ptr<visuals::DisplayItemList> display_item_list) = 0;

  virtual void Reconvert(WindowId window_id, const base::string16& text) = 0;
  virtual void RealizeWindow(WindowId window_id) = 0;

  // Release capture from window or form control.
  virtual void ReleaseCapture(EventTargetId event_target_id) = 0;

  // Release capture from window or form control.
  virtual void SetCapture(EventTargetId event_target_id) = 0;

  // Set text contents of status bar of specified top level window.
  virtual void SetStatusBar(WindowId window_id,
                            const std::vector<base::string16>& texts) = 0;

  // Set switch value
  virtual void SetSwitch(const base::string16& name,
                         const SwitchValue& new_value) = 0;

  virtual void SetTabData(WindowId window_id, const TabData& tab_data) = 0;
  virtual void ShowWindow(WindowId window_id) = 0;
  virtual void SplitHorizontally(WindowId left_window_id,
                                 WindowId new_right_window_id) = 0;
  virtual void SplitVertically(WindowId above_window_id,
                               WindowId new_below_window_id) = 0;

  // Trace Log
  virtual void StartTraceLog(const std::string& config) = 0;
  virtual void StopTraceLog(const TraceLogOutputCallback& callback) = 0;

 protected:
  ViewDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_VIEW_DELEGATE_H_
