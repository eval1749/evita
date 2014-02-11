// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/view_delegate_impl.h"

#include "base/logging.h"
#include "base/synchronization/waitable_event.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/text_window.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/dialog_box.h"
#include "evita/editor/dom_lock.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_FileDialogBox.h"
#include "evita/vi_FindDialogBox.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"
#include "evita/views/table_view.h"

namespace views {

namespace {

class WaitableEventScope {
  private: base::WaitableEvent* event_;

  public: WaitableEventScope(base::WaitableEvent* event) : event_(event) {
    DCHECK(!event->IsSignaled());
  }
  public: ~WaitableEventScope() {
    event_->Signal();
  }

  DISALLOW_COPY_AND_ASSIGN(WaitableEventScope);
};

Window* FromWindowId(const char* name, dom::WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << name << ": No such window " << window_id;
    return nullptr;
  }
  return window;
}

Frame* FindFrame(Window* window) {
  if (auto const frame = Frame::FindFrame(*window))
    return frame;
  return Application::instance()->GetActiveFrame();
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

void ViewDelegateImpl::ComputeOnTextWindow(dom::WindowId window_id,
                                         dom::TextWindowCompute* data,
                                         base::WaitableEvent* event) {
  WaitableEventScope waitable_event_scope(event);
  auto const window = FromWindowId("ComputeOnTextWindow", window_id)->
      as<TextEditWindow>();
  if (!window)
    return;
  UI_DOM_AUTO_LOCK_SCOPE();
  switch (data->method) {
    case dom::TextWindowCompute::Method::EndOfWindow:
      data->position = window->GetEnd();
      break;
    case dom::TextWindowCompute::Method::EndOfWindowLine:
      data->position = window->EndOfLine(data->position);
      break;
    case dom::TextWindowCompute::Method::MapPointToPosition:
     data->position = window->MapPointToPosn(gfx::PointF(data->x, data->y));
      break;
   case dom::TextWindowCompute::Method::MapPositionToPoint: {
      const auto rect = window->MapPosnToPoint(data->position);
      if (rect) {
        data->x = rect.left;
        data->y = rect.top;
      } else {
        data->x = data->y = -1.0f;
      }
      break;
    }
    case dom::TextWindowCompute::Method::MoveScreen: {
      gfx::PointF point(data->x, data->y);
      text::Posn position = data->position;
      window->ComputeMotion(Unit_Screen, data->count, point, &position);
      data->position = position;
      break;
    }
    case dom::TextWindowCompute::Method::MoveWindow: {
      gfx::PointF point(data->x, data->y);
      window->ComputeMotion(Unit_Window, data->count, point, &data->position);
      break;
    }
    case dom::TextWindowCompute::Method::MoveWindowLine: {
      gfx::PointF point(data->x, data->y);
      window->ComputeMotion(Unit_WindowLine, data->count, point,
                            &data->position);
      break;
    }
    case dom::TextWindowCompute::Method::StartOfWindow:
      data->position = window->GetStart();
      break;
    case dom::TextWindowCompute::Method::StartOfWindowLine:
      data->position = window->StartOfLine(data->position);
      break;
  }
}

void ViewDelegateImpl::CreateDialogBox(dom::DialogBoxId dialog_box_id) {
  new FindDialogBox(dialog_box_id);
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

void ViewDelegateImpl::DoFind(dom::DialogBoxId dialog_box_id,
                              text::Direction direction) {
  auto const dialog_box = DialogBox::FromDialogBoxId(dialog_box_id);
  if (!dialog_box)
    return;
  static_cast<FindDialogBox*>(dialog_box)->DoFind(direction);
}

void ViewDelegateImpl::FocusWindow(dom::WindowId window_id) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "FocusWindow: no such widget " << window_id;
    return;
  }
  widget->SetFocus();
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

void ViewDelegateImpl::GetTableRowStates(WindowId window_id,
    const std::vector<base::string16>& keys, int* states,
    base::WaitableEvent* event) {
  WaitableEventScope waitable_event_scope(event);
  auto const widget = FromWindowId("GetTableRowStates", window_id);
  if (!widget)
    return;
  auto const table_view = widget->as<views::TableView>();
  if (!table_view)
    return;
  table_view->GetRowStates(keys, states);
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

void ViewDelegateImpl::MessageBox(dom::WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) {
  auto const widget = window_id == dom::kInvalidWindowId ? nullptr :
      FromWindowId("MessageBoxCallback", window_id);
  auto const frame = widget ? FindFrame(widget) : nullptr;
  auto const response_code = frame ?
      frame->MessageBox(widget, message, title, flags) :
      ::MessageBoxW(nullptr, message.c_str(), title.c_str(),
                    static_cast<UINT>(flags));
  event_handler_->RunCallback(base::Bind(callback, response_code));
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
  text_window->Reconvert(start, end);
}

void ViewDelegateImpl::RealizeDialogBox(const dom::Form* form) {
  auto const dialog_box = DialogBox::FromDialogBoxId(form->event_target_id());
  if (!dialog_box)
    return;
  dialog_box->Realize(form);
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
    dom::ViewEventHandler* event_handler) {
  DCHECK(!event_handler_);
  event_handler_ = event_handler;
  event_handler_->DidStartHost();
}
void ViewDelegateImpl::SaveFile(dom::Document* document,
                                const base::string16& filename) {
  auto const buffer = document->buffer();
  auto const code_page = buffer->GetCodePage() ? buffer->GetCodePage() : 932;
  auto const newline_mode = buffer->GetNewline() == NewlineMode_Detect ?
        NewlineMode_CrLf : buffer->GetNewline();
  buffer->Save(filename.c_str(), code_page, newline_mode);
}

void ViewDelegateImpl::ShowDialogBox(dom::DialogBoxId dialog_box_id) {
  auto const dialog_box = DialogBox::FromDialogBoxId(dialog_box_id);
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
