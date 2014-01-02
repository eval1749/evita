// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/view_delegate_impl.h"

#include "base/logging.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/text_window.h"
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

void ViewDelegateImpl::DestroyWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
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
}

}  // namespace view
