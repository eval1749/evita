// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

namespace dom {

Window::Window(widgets::WidgetId widget_id) : widget_id_(widget_id) {
  DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
}

Window::~Window() {
  DCHECK_EQ(widgets::kInvalidWidgetId, widget_id_);
}

v8_glue::WrapperInfo* Window::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info, ("Window"));
  return &wrapper_info;
}

gin::ObjectTemplateBuilder Window::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetEmptyObjectTemplateBuilder(isolate)
    .SetProperty("id", &Window::id);
}

}  // namespace dom
