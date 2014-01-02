// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/view_delegate_impl.h"

#include "base/logging.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/text_window.h"
#include "evita/dom/view_event_handler.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"

namespace view {

ViewDelegateImpl::ViewDelegateImpl()
    : event_handler_(nullptr) {
}

ViewDelegateImpl::~ViewDelegateImpl() {
  // event_handler_ should be nullptr
}

void ViewDelegateImpl::CreateEditorWindow(const dom::EditorWindow* window) {
  DCHECK(window);
  new Frame(window->widget_id());
}

void ViewDelegateImpl::CreateTextWindow(const dom::TextWindow* window) {
  DCHECK(window);
  new TextEditWindow(*window);
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

void ViewDelegateImpl::DestroyWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
  widgets::Widget::DidDestroyDomWindow(widget_id);
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

}  // namespace view
