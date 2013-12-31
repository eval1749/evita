// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/view_delegate_impl.h"

#include "base/logging.h"

namespace view {

ViewDelegateImpl::ViewDelegateImpl()
    : event_handler_(nullptr) {
}

ViewDelegateImpl::~ViewDelegateImpl() {
  // event_handler_ should be nullptr
}

void ViewDelegateImpl::CreateEditorWindow(const dom::EditorWindow* window) {
  DCHECK(window);
}

void ViewDelegateImpl::CreateTextWindow(const dom::TextWindow* window) {
  DCHECK(window);
}

void ViewDelegateImpl::DestroyWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
}

void ViewDelegateImpl::RealizeWindow(dom::WidgetId widget_id) {
  DCHECK_NE(dom::kInvalidWidgetId, widget_id);
}

void ViewDelegateImpl::RegisterViewEventHandler(
    dom::ViewEventHandler* event_handler) {
  DCHECK(!event_handler_);
  event_handler_ = event_handler;
}

}  // namespace view
