// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/view_delegate_impl.h"

#include <sstream>
#include <string>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/public/cursor.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/text_area_display_item.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/text_window.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/modal_message_loop_scope.h"
#include "evita/editor/scheduler.h"
#include "evita/editor/switch_set.h"
#include "evita/editor/trace_log_controller.h"
#include "evita/frames/edit_pane.h"
#include "evita/frames/frame.h"
#include "evita/frames/frame_list.h"
#include "evita/gc/collector.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/resource.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/text_field_control.h"
#include "evita/views/forms/file_dialog_box.h"
#include "evita/views/forms/form_window.h"
#include "evita/views/tabs/tab_data_set.h"
#include "evita/views/text_window.h"
#include "evita/views/visual_window.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

namespace {

domapi::ViewEventHandler* ScriptDelegate() {
  return editor::Application::instance()->view_event_handler();
}

template <typename ResolveType>
void Reject(const domapi::Promise<ResolveType, int>& promise, int value) {
  ScriptDelegate()->RunCallback(base::Bind(promise.reject, value));
}

void Resolve(const domapi::IntegerPromise& promise, int value) {
  ScriptDelegate()->RunCallback(base::Bind(promise.resolve, value));
}

//////////////////////////////////////////////////////////////////////
//
// TraceLogClient
//
class TraceLogClient final {
 public:
  explicit TraceLogClient(const domapi::TraceLogOutputCallback& callback)
      : callback_(callback) {}
  ~TraceLogClient() = default;

  void DidGetEvent(const std::string& chunk, bool has_more_events);

 private:
  domapi::TraceLogOutputCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(TraceLogClient);
};

void TraceLogClient::DidGetEvent(const std::string& chunk,
                                 bool has_more_events) {
  ScriptDelegate()->RunCallback(base::Bind(callback_, chunk, has_more_events));
  if (has_more_events)
    return;
  delete this;
}

Window* FromWindowId(const char* name, domapi::WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << name << ": No such window " << window_id;
    return nullptr;
  }
  return window;
}

Frame* GetFrameForMessage(domapi::WindowId window_id) {
  if (window_id == domapi::kInvalidWindowId)
    return FrameList::instance()->active_frame();
  auto const window = FromWindowId("GetFrameForMessage", window_id);
  if (!window)
    return FrameList::instance()->active_frame();
  for (auto runner = static_cast<ui::Widget*>(window); runner;
       runner = runner->parent_node()) {
    if (auto const frame = runner->as<Frame>())
      return frame;
  }
  return FrameList::instance()->active_frame();
}

}  // namespace

ViewDelegateImpl::ViewDelegateImpl() {}

ViewDelegateImpl::~ViewDelegateImpl() {}

void ViewDelegateImpl::AddWindow(domapi::WindowId parent_id,
                                 domapi::WindowId child_id) {
  DCHECK_NE(domapi::kInvalidWindowId, parent_id);
  auto const parent = Window::FromWindowId(parent_id);
  if (!parent) {
    DVLOG(0) << "AddWindow: no such parent " << parent_id;
    return;
  }
  DCHECK_EQ(parent_id, parent->window_id());
  auto const child = Window::FromWindowId(child_id);
  if (!child) {
    DVLOG(0) << "AddWindow: no such child " << child_id;
    return;
  }
  DCHECK_EQ(child_id, child->window_id());
  parent->as<Frame>()->AddOrActivateTabContent(
      child->as<views::ContentWindow>());
}

void ViewDelegateImpl::ChangeParentWindow(domapi::WindowId window_id,
                                          domapi::WindowId new_parent_id) {
  auto const window = FromWindowId("ChangeParentWindow", window_id);
  if (!window)
    return;
  auto const new_parent = FromWindowId("ChangeParentWindow", new_parent_id);
  if (!new_parent)
    return;
  window->SetParentWidget(new_parent);
}

void ViewDelegateImpl::CreateEditorWindow(domapi::WindowId window_id) {
  new Frame(window_id);
}

void ViewDelegateImpl::CreateFormWindow(domapi::WindowId window_id,
                                        domapi::WindowId owner_window_id,
                                        const domapi::IntRect& bounds,
                                        const base::string16& title) {
  if (owner_window_id == domapi::kInvalidWindowId) {
    new FormWindow(window_id, nullptr, bounds, title);
    return;
  }
  auto const owner = Window::FromWindowId(owner_window_id);
  if (!owner) {
    DVLOG(0) << "CreateFormWindow: no such window " << owner_window_id;
    return;
  }
  new FormWindow(window_id, owner, bounds, title);
}

void ViewDelegateImpl::CreateTextWindow(domapi::WindowId window_id) {
  new TextWindow(window_id);
}

void ViewDelegateImpl::CreateVisualWindow(domapi::WindowId window_id) {
  new VisualWindow(window_id);
}

void ViewDelegateImpl::DestroyWindow(domapi::WindowId window_id) {
  DCHECK_NE(domapi::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "DestroyWindow: no such widget " << window_id;
    return;
  }
  widget->DidDestroyDomWindow();
  widget->DestroyWidget();
}

void ViewDelegateImpl::DidStartScriptHost(domapi::ScriptHostState state) {
  editor::Application::instance()->DidStartScriptHost(state);
}

void ViewDelegateImpl::FocusWindow(domapi::WindowId window_id) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "FocusWindow: no such widget " << window_id;
    return;
  }
  widget->RequestFocus();
}

void ViewDelegateImpl::GetFileNameForLoad(
    domapi::WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForLoadResolver& resolver) {
  TRACE_EVENT_WITH_FLOW0("promise", "ViewDelegateImpl::GetFileNameForLoad",
                         resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  auto const widget = window_id == domapi::kInvalidWindowId
                          ? FrameList::instance()->active_frame()
                          : Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFileNameForLoad: no such widget " << window_id;
    ScriptDelegate()->RunCallback(
        base::Bind(resolver.resolve, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetOpenFileName(&params))
    return;
  ScriptDelegate()->RunCallback(
      base::Bind(resolver.resolve, base::string16(params.m_wsz)));
}

void ViewDelegateImpl::GetFileNameForSave(
    domapi::WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForSaveResolver& resolver) {
  TRACE_EVENT_WITH_FLOW0("promise", "ViewDelegateImpl::GetFileNameForSave",
                         resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  auto const widget = window_id == domapi::kInvalidWindowId
                          ? FrameList::instance()->active_frame()
                          : Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFileNameForSave: no such widget " << window_id;
    ScriptDelegate()->RunCallback(
        base::Bind(resolver.resolve, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetSaveFileName(&params))
    return;
  ScriptDelegate()->RunCallback(
      base::Bind(resolver.resolve, base::string16(params.m_wsz)));
}

void ViewDelegateImpl::GetMetrics(const base::string16& name,
                                  const domapi::StringPromise& promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "ViewDelegateImpl::GetMetrics",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "name", base::UTF16ToUTF8(name));
  UI_DOM_AUTO_LOCK_SCOPE();
  base::string16 delimiter = L"";
  const base::string16 comma = L",\n";

  std::basic_ostringstream<base::char16> ostream;
  ostream << '{';

  auto const times = metrics::HistogramSet::instance()->GetJson(name);
  if (!times.empty()) {
    ostream << delimiter << L"\"times\": " << times;
    delimiter = comma;
  }

  auto const counters = metrics::CounterSet::instance()->GetJson(name);
  if (!counters.empty()) {
    ostream << delimiter << L"\"counters\": " << counters;
    delimiter = comma;
  }

  auto const objects = gc::Collector::instance()->GetJson(name);
  if (!objects.empty()) {
    ostream << delimiter << L"\"objects\": " << objects;
    delimiter = comma;
  }

  ostream << '}';
  ScriptDelegate()->RunCallback(base::Bind(promise.resolve, ostream.str()));
}

// TODO(eval1749): We should make |GetSwitch()| to return value asynchronously.
domapi::SwitchValue ViewDelegateImpl::GetSwitch(const base::string16& name) {
  TRACE_EVENT0("view", "ViewDelegateImpl::GetSwitch");
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return editor::SwitchSet::instance()->Get(name);
}

// TODO(eval1749): We should make |GetSwitchNames()| to return value
// asynchronously.
std::vector<base::string16> ViewDelegateImpl::GetSwitchNames() {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return editor::SwitchSet::instance()->names();
}

void ViewDelegateImpl::HideWindow(domapi::WindowId window_id) {
  auto const window = FromWindowId("HideWindow", window_id);
  if (!window)
    return;
  window->Hide();
}

void ViewDelegateImpl::MessageBox(domapi::WindowId window_id,
                                  const base::string16& message,
                                  const base::string16& title,
                                  int flags,
                                  const MessageBoxResolver& resolver) {
  TRACE_EVENT_WITH_FLOW0("promise", "ViewDelegateImpl::MessageBox",
                         resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  auto const frame = GetFrameForMessage(window_id);

  auto const kButtonMask = 7;
  auto const kIconMask = 0x70;
  auto const need_response = flags & kButtonMask;
  auto const level = (flags & kIconMask) == MB_ICONERROR
                         ? MessageLevel_Error
                         : (flags & kIconMask) == MB_ICONWARNING
                               ? MessageLevel_Warning
                               : MessageLevel_Information;

  if (!need_response && level != MessageLevel_Error) {
    if (frame)
      frame->ShowMessage(level, message);
    ScriptDelegate()->RunCallback(base::Bind(resolver.resolve, IDOK));
    return;
  }

  auto safe_title = title;
  if (!safe_title.empty())
    safe_title += L" - ";
  safe_title += editor::Application::instance()->title();
  auto const hwnd = frame ? frame->AssociatedHwnd() : nullptr;
  editor::ModalMessageLoopScope modal_message_loop_scope;
  auto const response = ::MessageBoxW(hwnd, message.c_str(), title.c_str(),
                                      static_cast<UINT>(flags));
  ScriptDelegate()->RunCallback(base::Bind(resolver.resolve, response));
}

void ViewDelegateImpl::PaintForm(domapi::WindowId window_id,
                                 std::unique_ptr<domapi::Form> form) {
  auto const& window = FromWindowId("PaintForm", window_id);
  if (!window)
    return;
  auto const& form_window = window->as<FormWindow>();
  if (!form_window) {
    DVLOG(0) << "WindowId " << window_id << " should be FormWindow.";
    return;
  }
  form_window->Paint(std::move(form));
}

void ViewDelegateImpl::PaintTextArea(
    domapi::WindowId window_id,
    std::unique_ptr<domapi::TextAreaDisplayItem> display_item) {
  auto const& window = FromWindowId("PaintTextArea", window_id);
  if (!window)
    return;
  auto const& text_window = window->as<TextWindow>();
  if (!text_window) {
    DVLOG(0) << "WindowId " << window_id << " should be TextAreaWindow.";
    return;
  }
  text_window->Paint(std::move(display_item));
}

void ViewDelegateImpl::PaintVisualDocument(
    domapi::WindowId window_id,
    std::unique_ptr<visuals::DisplayItemList> display_item_list) {
  auto const& window = FromWindowId("PaintVisualDocument", window_id);
  if (!window)
    return;
  auto const& visual_window = window->as<VisualWindow>();
  if (!visual_window) {
    DVLOG(0) << "WindowId " << window_id << " should be VisualWindow.";
    return;
  }
  visual_window->Paint(std::move(display_item_list));
}

void ViewDelegateImpl::Reconvert(WindowId window_id,
                                 const base::string16& text) {
  auto const window = FromWindowId("Reconvert", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextWindow>();
  if (!text_window) {
    DVLOG(0) << "WindowId " << window_id << " should be TextWindow.";
    return;
  }
  ui::TextInputClient::Get()->Reconvert(text_window, text);
}

void ViewDelegateImpl::RealizeWindow(domapi::WindowId window_id) {
  DCHECK_NE(domapi::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget)
    return;
  DCHECK_EQ(window_id, widget->window_id());
  widget->RealizeWidget();
}

void ViewDelegateImpl::ShowWindow(domapi::WindowId window_id) {
  auto const window = FromWindowId("ShowWindow", window_id);
  if (!window)
    return;
  window->Show();
}

void ViewDelegateImpl::ReleaseCapture(domapi::EventTargetId event_target_id) {
  if (auto const window = Window::FromWindowId(event_target_id)) {
    window->ReleaseCapture();
    return;
  }

  DVLOG(0) << "ReleaseCapture: no such target " << event_target_id;
}

void ViewDelegateImpl::SetCapture(domapi::EventTargetId event_target_id) {
  if (auto const window = Window::FromWindowId(event_target_id)) {
    window->SetCapture();
    return;
  }

  DVLOG(0) << "SetCapture: no such target " << event_target_id;
}

void ViewDelegateImpl::SetCursor(domapi::WindowId window_id,
                                 domapi::CursorId cursor_id) {
  const auto window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << "SetCursor: no such window " << window_id;
    return;
  }

// See "ui/base/cursor/cursor_loader_win.cc"
#define IDC_Alias IDC_ARROW
#define IDC_Cell MAKEINTRESOURCE(IDC_CELL)
#define IDC_ColumnResize MAKEINTRESOURCE(IDC_COLRESIZE)
#define IDC_ContextMenu IDC_ARROW
#define IDC_Copy MAKEINTRESOURCE(IDC_COPYCUR)
#define IDC_Cross IDC_CROSS
#define IDC_Custom IDC_ARROW
#define IDC_EastPanning MAKEINTRESOURCE(IDC_PAN_EAST)
#define IDC_EastResize IDC_SIZEWE
#define IDC_EastWestResize IDC_SIZEWE
#define IDC_Grab MAKEINTRESOURCE(IDC_HAND_GRAB)
#define IDC_Grabbing MAKEINTRESOURCE(IDC_HAND_GRABBING)
#define IDC_Hand IDC_HAND
#define IDC_Help IDC_HELP
#define IDC_IBeam IDC_IBEAM
#define IDC_MiddlePanning MAKEINTRESOURCE(IDC_PAN_MIDDLE)
#define IDC_Move IDC_SIZEALL
#define IDC_NoDrop IDC_NO
#define IDC_None IDC_NO
#define IDC_NorthEastPanning MAKEINTRESOURCE(IDC_PAN_NORTH_EAST)
#define IDC_NorthEastResize IDC_SIZENESW
#define IDC_NorthEastSouthWestResize IDC_SIZENESW
#define IDC_NorthPanning MAKEINTRESOURCE(IDC_PAN_NORTH)
#define IDC_NorthResize IDC_SIZENS
#define IDC_NorthSouthResize IDC_SIZENS
#define IDC_NorthWestSouthEastResize MAKEINTRESOURCE(IDC_PAN_SOUTH_EAST)
#define IDC_NorthWestPanning MAKEINTRESOURCE(IDC_PAN_WEST)
#define IDC_NorthWestResize IDC_SIZENWSE
#define IDC_NotAllowed IDC_NO
#define IDC_Pointer IDC_ARROW
#define IDC_Progress IDC_APPSTARTING
#define IDC_RowResize MAKEINTRESOURCE(IDC_ROWRESIZE)
#define IDC_SouthEastPanning MAKEINTRESOURCE(IDC_PAN_SOUTH_EAST)
#define IDC_SouthEastResize IDC_SIZENWSE
#define IDC_SouthPanning MAKEINTRESOURCE(IDC_PAN_SOUTH)
#define IDC_SouthResize IDC_SIZENS
#define IDC_SouthWestPanning MAKEINTRESOURCE(IDC_PAN_SOUTH_WEST)
#define IDC_SouthWestResize IDC_SIZENESW
#define IDC_VerticalText MAKEINTRESOURCE(IDC_VERTICALTEXT)
#define IDC_Wait IDC_WAIT
#define IDC_WestPanning MAKEINTRESOURCE(IDC_PAN_WEST)
#define IDC_WestResize IDC_SIZEWE
#define IDC_ZoomIn MAKEINTRESOURCE(IDC_ZOOMIN)
#define IDC_ZoomOut MAKEINTRESOURCE(IDC_ZOOMOUT)

  static LPCWSTR names[] = {
#define V(Name) IDC_##Name,
      FOR_EACH_DOMAPI_CURSOR(V)
#undef V
  };
  const auto& it = std::begin(names) + static_cast<size_t>(cursor_id);
  if (it < std::begin(names) || it >= std::end(names)) {
    DVLOG(0) << "SetCursor but cursor id " << cursor_id;
    return;
  }
  const auto cursor = ::LoadCursor(nullptr, *it);
  if (!cursor) {
    const auto last_error = ::GetLastError();
    DVLOG(0) << "LoadCursor error=" << last_error;
    return;
  }
  window->SetCursor(cursor);
}

void ViewDelegateImpl::SetStatusBar(domapi::WindowId window_id,
                                    const std::vector<base::string16>& texts) {
  auto const window = FromWindowId("SetStatusBar", window_id);
  if (!window)
    return;
  auto const frame = window->as<Frame>();
  if (!frame) {
    DVLOG(0) << "Window " << window_id << " should be Frame window.";
    return;
  }
  frame->SetStatusBar(texts);
}

void ViewDelegateImpl::SetSwitch(const base::string16& name,
                                 const domapi::SwitchValue& new_value) {
  TRACE_EVENT0("view", "ViewDelegateImpl::SetSwitch");
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  editor::SwitchSet::instance()->Set(name, new_value);
}

void ViewDelegateImpl::SetTabData(domapi::WindowId window_id,
                                  const domapi::TabData& tab_data) {
  TabDataSet::instance()->SetTabData(window_id, tab_data);
}

void ViewDelegateImpl::SplitHorizontally(domapi::WindowId left_window_id,
                                         domapi::WindowId new_right_window_id) {
  auto const left_window = Window::FromWindowId(left_window_id);
  if (!left_window)
    return;
  auto const new_right_window = Window::FromWindowId(new_right_window_id);
  if (!new_right_window)
    return;
  auto const parent = left_window->parent_node()->as<EditPane>();
  if (!parent)
    return;
  parent->SplitHorizontally(left_window->as<ContentWindow>(),
                            new_right_window->as<ContentWindow>());
}

void ViewDelegateImpl::SplitVertically(domapi::WindowId above_window_id,
                                       domapi::WindowId new_below_window_id) {
  auto const above_window = Window::FromWindowId(above_window_id);
  if (!above_window)
    return;
  auto const new_below_window = Window::FromWindowId(new_below_window_id);
  if (!new_below_window)
    return;
  auto const parent = above_window->parent_node()->as<EditPane>();
  if (!parent)
    return;
  parent->SplitVertically(above_window->as<ContentWindow>(),
                          new_below_window->as<ContentWindow>());
}

void ViewDelegateImpl::StartTraceLog(const std::string& config) {
  editor::Application::instance()->trace_log_controller()->StartRecording(
      config);
}

void ViewDelegateImpl::StopTraceLog(
    const domapi::TraceLogOutputCallback& callback) {
  auto const trace_log_client = new TraceLogClient(callback);
  editor::Application::instance()->trace_log_controller()->StopRecording(
      base::Bind(&TraceLogClient::DidGetEvent,
                 base::Unretained(trace_log_client)));
}

}  // namespace views
