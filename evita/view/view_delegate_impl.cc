// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/view_delegate_impl.h"

#include "base/logging.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/text_window.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/dom_lock.h"
#include "evita/vi_FileDialogBox.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"

namespace view {

ViewDelegateImpl::ViewDelegateImpl()
    : event_handler_(nullptr) {
}

ViewDelegateImpl::~ViewDelegateImpl() {
  // event_handler_ should be nullptr
}

void ViewDelegateImpl::AddWindow(dom::WidgetId parent_id,
                                 dom::WidgetId child_id) {
  DCHECK_NE(dom::kInvalidWidgetId, parent_id);
  auto const parent = widgets::Widget::FromWidgetId(parent_id);
  if (!parent) {
    DVLOG(0) << "AddWindow: no such parent " << parent_id;
    return;
  }
  DCHECK_EQ(parent_id, parent->widget_id());
  auto const child = widgets::Widget::FromWidgetId(child_id);
  if (!child) {
    DVLOG(0) << "AddWindow: no such child " << child_id;
    return;
  }
  DCHECK_EQ(child_id, child->widget_id());
  parent->as<Frame>()->AddWindow(child->as<content::ContentWindow>());
}

void ViewDelegateImpl::CreateEditorWindow(const dom::EditorWindow* window) {
  DCHECK(window);
  new Frame(window->widget_id());
}

void ViewDelegateImpl::CreateTextWindow(const dom::TextWindow* window) {
  DCHECK(window);
  new TextEditWindow(*window);
}

void ViewDelegateImpl::DestroyWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget) {
    DVLOG(0) << "DestroyWindow: no such widget " << widget_id;
    return;
  }
  widget->DidDestroyDomWindow();
  widget->Destroy();
}

void ViewDelegateImpl::FocusWindow(dom::WidgetId widget_id) {
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget) {
    DVLOG(0) << "FocusWindow: no such widget " << widget_id;
    return;
  }
  widget->SetFocus();
}

void ViewDelegateImpl::GetFilenameForLoad(
    dom::WidgetId widget_id, const base::string16& dir_path,
    GetFilenameForLoadCallback callback) {
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget) {
    DVLOG(0) << "GetFilenameForLoad: no such widget " << widget_id;
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
                                         base::string16(params.m_pwszFile)));
}

void ViewDelegateImpl::GetFilenameForSave(
    dom::WidgetId widget_id, const base::string16& dir_path,
    GetFilenameForSaveCallback callback) {
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget) {
    DVLOG(0) << "GetFilenameForSave: no such widget " << widget_id;
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
                                         base::string16(params.m_pwszFile)));
}

void ViewDelegateImpl::LoadFile(dom::Document* document,
                                const base::string16& filename) {
  document->buffer()->Load(filename.c_str());
}

void ViewDelegateImpl::MakeSelectionVisible(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget) {
    DVLOG(0) << "MakeSelectionVisible: no such widget " << widget_id;
    return;
  }
  auto const text_widget = widget->as<TextEditWindow>();
  if (!text_widget) {
    DVLOG(0) << "MakeSelectionVisible: not TextWidget " << widget_id;
    return;
  }
  UI_DOM_AUTO_LOCK_SCOPE();
  text_widget->MakeSelectionVisible();
}

void ViewDelegateImpl::RealizeWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
  auto const widget = widgets::Widget::FromWidgetId(widget_id);
  if (!widget)
    return;
  DCHECK_EQ(widget_id, widget->widget_id());
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

}  // namespace view
