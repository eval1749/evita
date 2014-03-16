// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/view_delegate_impl.h"

#include <sstream>

#include "base/logging.h"
// TODO(yosi) We should not use |dom::Buffer| outside DOM.
#include "evita/dom/text/buffer.h"
#include "evita/dom/text/document.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/windows/text_window.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/modal_message_loop_scope.h"
#include "evita/gc/collector.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/spellchecker/spelling_engine.h"
#include "evita/views/forms/dialog_box_set.h"
#include "evita/views/forms/file_dialog_box.h"
#include "evita/views/forms/find_dialog_box.h"
#include "evita/views/frame_list.h"
#include "evita/views/table_view.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"

namespace views {

namespace {

Window* FromWindowId(const char* name, dom::WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << name << ": No such window " << window_id;
    return nullptr;
  }
  return window;
}

Frame* GetFrameForMessage(dom::WindowId window_id) {
  if (window_id == dom::kInvalidWindowId)
    return FrameList::instance()->active_frame();
  auto const window = FromWindowId("GetFrameForMessage", window_id);
  if (!window)
    return FrameList::instance()->active_frame();
  if (auto const frame = Frame::FindFrame(*window))
    return frame;
  return FrameList::instance()->active_frame();
}

}  // namespace

ViewDelegateImpl::ViewDelegateImpl()
    : event_handler_(nullptr) {
}

ViewDelegateImpl::~ViewDelegateImpl() {
  // event_handler_ should be nullptr
}

void ViewDelegateImpl::AddWindow(dom::WindowId parent_id,
                                 dom::WindowId child_id) {
  DCHECK_NE(dom::kInvalidWindowId, parent_id);
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
  parent->as<Frame>()->AddWindow(child->as<views::ContentWindow>());
}

void ViewDelegateImpl::ChangeParentWindow(dom::WindowId window_id,
                                          dom::WindowId new_parent_id) {
  auto const window = FromWindowId("ChangeParentWindow", window_id);
  if (!window)
    return;
  auto const new_parent = FromWindowId("ChangeParentWindow", new_parent_id);
  if (!new_parent)
    return;
  window->SetParentWidget(new_parent);
}

void ViewDelegateImpl::CheckSpelling(const base::string16& word_to_check,
    const CheckSpellingDeferred& deferred) {
  event_handler_->RunCallback(base::Bind(deferred.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->
          CheckSpelling(word_to_check)));
}

text::Posn ViewDelegateImpl::ComputeOnTextWindow(
    dom::WindowId window_id, const dom::TextWindowCompute& data) {
  auto const window = FromWindowId("ComputeOnTextWindow", window_id)->
      as<TextEditWindow>();
  if (!window)
    return -1;
  gfx::PointF point(data.x, data.y);
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  switch (data.method) {
    case dom::TextWindowCompute::Method::EndOfWindow:
      return window->GetEnd();
    case dom::TextWindowCompute::Method::EndOfWindowLine:
      return window->EndOfLine(data.position);
    case dom::TextWindowCompute::Method::MoveScreen:
      return window->ComputeMotion(Unit_Screen, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::MoveWindow:
      return window->ComputeMotion(Unit_Window, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::MoveWindowLine:
      return window->ComputeMotion(Unit_WindowLine, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::StartOfWindow:
      return window->GetStart();
    case dom::TextWindowCompute::Method::StartOfWindowLine:
      return window->StartOfLine(data.position);
    default:
      return -1;
  }
}

void ViewDelegateImpl::CreateDialogBox(dom::Form* form) {
  new FindDialogBox(form);
}

void ViewDelegateImpl::CreateEditorWindow(const dom::EditorWindow* window) {
  DCHECK(window);
  new Frame(window->window_id());
}

void ViewDelegateImpl::CreateTableWindow(dom::WindowId window_id,
                                         dom::Document* document) {
  new views::TableView(window_id, document);
}


void ViewDelegateImpl::CreateTextWindow(const dom::TextWindow* window) {
  DCHECK(window);
  new TextEditWindow(*window);
}

void ViewDelegateImpl::DestroyWindow(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "DestroyWindow: no such widget " << window_id;
    return;
  }
  widget->DidDestroyDomWindow();
  widget->DestroyWidget();
}

void ViewDelegateImpl::DidChangeFormContents(
    domapi::DialogBoxId dialog_box_id) {
  auto const dialog_box = DialogBoxSet::instance()->Find(dialog_box_id);
  if (!dialog_box) {
    DVLOG(0) << "No such dialog box " << dialog_box_id;
    return;
  }
  dialog_box->DidChangeFormContents();
}

void ViewDelegateImpl::DidStartScriptHost(domapi::ScriptHostState state) {
  Application::instance()->DidStartScriptHost(state);
}

void ViewDelegateImpl::DidHandleViewIdelEvent(int hint) {
  Application::instance()->DidHandleViewIdelEvent(hint);
}

void ViewDelegateImpl::FocusWindow(dom::WindowId window_id) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "FocusWindow: no such widget " << window_id;
    return;
  }
  widget->RequestFocus();
}

void ViewDelegateImpl::GetFilenameForLoad(
    dom::WindowId window_id, const base::string16& dir_path,
    GetFilenameForLoadCallback callback) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFilenameForLoad: no such widget " << window_id;
    event_handler_->RunCallback(base::Bind(callback, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetOpenFileName(&params))
    return;
  event_handler_->RunCallback(base::Bind(callback,
                                         base::string16(params.m_wsz)));
}

void ViewDelegateImpl::GetFilenameForSave(
    dom::WindowId window_id, const base::string16& dir_path,
    GetFilenameForSaveCallback callback) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFilenameForSave: no such widget " << window_id;
    event_handler_->RunCallback(base::Bind(callback, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetSaveFileName(&params))
    return;
  event_handler_->RunCallback(base::Bind(callback,
                                         base::string16(params.m_wsz)));
}

base::string16 ViewDelegateImpl::GetMetrics(const base::string16& name) {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());

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
  return ostream.str();
}

void ViewDelegateImpl::GetSpellingSuggestions(const base::string16& wrong_word,
    const GetSpellingSuggestionsDeferred& deferred) {
  event_handler_->RunCallback(base::Bind(deferred.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->
          GetSpellingSuggestions(wrong_word)));
}

std::vector<int> ViewDelegateImpl::GetTableRowStates(WindowId window_id,
    const std::vector<base::string16>& keys) {
  auto const widget = FromWindowId("GetTableRowStates", window_id);
  if (!widget)
    return std::vector<int>();
  auto const table_view = widget->as<views::TableView>();
  if (!table_view)
    return std::vector<int>();
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return std::move(table_view->GetRowStates(keys));
}

void ViewDelegateImpl::LoadFile(dom::Document* document,
                                const base::string16& filename,
                                LoadFileCallback callback) {
  document->buffer()->Load(filename.c_str(), callback);
}

void ViewDelegateImpl::MakeSelectionVisible(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "MakeSelectionVisible: no such widget " << window_id;
    return;
  }
  auto const text_widget = widget->as<TextEditWindow>();
  if (!text_widget) {
    DVLOG(0) << "MakeSelectionVisible: not TextWidget " << window_id;
    return;
  }
  UI_DOM_AUTO_LOCK_SCOPE();
  text_widget->MakeSelectionVisible();
}

text::Posn ViewDelegateImpl::MapPointToPosition(WindowId window_id,
                                                float x, float y) {
  auto const window = FromWindowId("ComputeOnTextWindow", window_id)->
      as<TextEditWindow>();
  if (!window)
    return 0;
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return window->MapPointToPosn(gfx::PointF(x, y));
}

domapi::FloatPoint ViewDelegateImpl::MapPositionToPoint(
    WindowId window_id, text::Posn position) {
  auto const window = FromWindowId("ComputeOnTextWindow", window_id)->
      as<TextEditWindow>();
  if (!window)
    return domapi::FloatPoint(-1.0f, -1.0f);
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  auto const rect = window->MapPosnToPoint(position);
  return rect ? domapi::FloatPoint(rect.left, rect.top) :
                domapi::FloatPoint(-1.0f, -1.0f);
}

void ViewDelegateImpl::MessageBox(dom::WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) {
  auto const frame = GetFrameForMessage(window_id);

  auto const kButtonMask = 7;
  auto const kIconMask = 0x70;
  auto const need_response = flags & kButtonMask;
  auto const level = (flags & kIconMask) == MB_ICONERROR ?
      MessageLevel_Error :
      (flags & kIconMask) == MB_ICONWARNING ? MessageLevel_Warning :
          MessageLevel_Information;

  if (!need_response && level != MessageLevel_Error) {
    if (frame)
      frame->ShowMessage(level, message);
    event_handler_->RunCallback(base::Bind(callback, IDOK));
    return;
  }

  auto safe_title = title;
  if (!safe_title.empty())
    safe_title += L" - ";
  safe_title += Application::instance()->title();
  auto const hwnd = frame ? frame->AssociatedHwnd() : nullptr;
  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  auto const response= ::MessageBoxW(hwnd, message.c_str(), title.c_str(),
                                     static_cast<UINT>(flags));
  event_handler_->RunCallback(base::Bind(callback, response));
}

void ViewDelegateImpl::Reconvert(WindowId window_id, text::Posn start,
                                 text::Posn end) {
  auto const window = FromWindowId("Reconvert", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextEditWindow>();
  if (!text_window) {
    DVLOG(0) << "WindowId " << window_id << " should be TextEditWindow.";
    return;
  }
  UI_DOM_AUTO_LOCK_SCOPE();
  text_window->Reconvert(start, end);
}

void ViewDelegateImpl::RealizeDialogBox(domapi::DialogBoxId dialog_box_id) {
  auto const dialog_box = DialogBoxSet::instance()->Find(dialog_box_id);
  if (!dialog_box) {
    DVLOG(0) << "No such dialog box " << dialog_box_id;
    return;
  }
  dialog_box->Realize();
}

void ViewDelegateImpl::RealizeWindow(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget)
    return;
  DCHECK_EQ(window_id, widget->window_id());
  widget->RealizeWidget();
}

void ViewDelegateImpl::RegisterViewEventHandler(
    domapi::ViewEventHandler* event_handler) {
  DCHECK(!event_handler_);
  event_handler_ = event_handler;
  event_handler_->DidStartViewHost();
}
void ViewDelegateImpl::SaveFile(dom::Document* document,
                                const base::string16& filename,
                                const SaveFileCallback& callback) {
  auto const buffer = document->buffer();
  auto const code_page = buffer->GetCodePage() ?
      static_cast<int>(buffer->GetCodePage()) : 932;
  auto const newline_mode = buffer->GetNewline() == NewlineMode_Detect ?
        NewlineMode_CrLf : buffer->GetNewline();
  buffer->Save(filename, code_page, newline_mode, callback);
}

void ViewDelegateImpl::ShowDialogBox(domapi::DialogBoxId dialog_box_id) {
  auto const dialog_box = DialogBoxSet::instance()->Find(dialog_box_id);
  if (!dialog_box)
    return;
  dialog_box->Show();
}

void ViewDelegateImpl::ReleaseCapture(dom::WindowId window_id) {
  auto const window = FromWindowId("ReleaseCapture", window_id);
  if (!window)
    return;
  window->ReleaseCapture();
}

void ViewDelegateImpl::ScrollTextWindow(WindowId window_id, int direction) {
  auto const window = FromWindowId("ScrollTextWindow", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextEditWindow>();
  if (!text_window) {
    DVLOG(0) << "ScrollTextWindow expects TextEditWindow.";
  }
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  text_window->SmallScroll(0, direction);
}

void ViewDelegateImpl::SetCapture(dom::WindowId window_id) {
  auto const window = FromWindowId("SetCapture", window_id);
  if (!window)
    return;
  window->SetCapture();
}

void ViewDelegateImpl::SplitHorizontally(dom::WindowId left_window_id,
    dom::WindowId new_right_window_id) {
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

void ViewDelegateImpl::SplitVertically(dom::WindowId above_window_id,
    dom::WindowId new_below_window_id) {
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

}  // namespace views
