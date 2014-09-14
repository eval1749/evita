// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/content_observer.h"

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

void ContentWindow::AddObserver(ContentObserver* observer) {
  observers_.AddObserver(observer);
}

void ContentWindow::NotifyUpdateContent() {
  FOR_EACH_OBSERVER(ContentObserver, observers_, DidUpdateContent(this));
}

void ContentWindow::RemoveObserver(ContentObserver* observer) {
  observers_.RemoveObserver(observer);
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
  FOR_EACH_OBSERVER(ContentObserver, observers_, DidActivateContent(this));
}

}  // namespace views
