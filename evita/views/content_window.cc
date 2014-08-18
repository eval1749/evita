// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/ui/compositor/layer.h"
#include "evita/vi_Frame.h"

namespace views {

ContentWindow::ContentWindow(views::WindowId window_id)
    : Window(window_id) {
}

ContentWindow::~ContentWindow() {
}

void ContentWindow::Activate() {
  #if DEBUG_FOCUS
    DVLOG(0) << "Activate " << this << " focus=" << has_focus() <<
        "show=" << visible();
  #endif
  RequestFocus();
}

Frame* ContentWindow::GetFrame() const {
  for (auto runner = static_cast<const Widget*>(this); runner;
       runner = &runner->container_widget()) {
    if (runner->is<Frame>())
      return const_cast<Frame*>(runner->as<Frame>());
  }
  NOTREACHED();
  return nullptr;
}

// ui::Widget
void ContentWindow::DidChangeBounds() {
  Window::DidChangeBounds();
  if (!layer())
    return;
  layer()->SetBounds(bounds());
}

void ContentWindow::DidChangeHierarchy() {
  Window::DidChangeHierarchy();
  container_widget().layer()->AppendLayer(layer());
}

void ContentWindow::DidRealize() {
  Window::DidRealize();
  SetLayer(new ui::Layer());
}

void ContentWindow::DidSetFocus(ui::Widget* widget) {
  Window::DidSetFocus(widget);
  if (auto const frame = GetFrame())
    frame->DidSetFocusOnChild(this);
}

}  // namespace views
