// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/ui/base/selection_state.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/content_observer.h"
#include "evita/views/frame_list.h"
#include "evita/vi_Frame.h"

namespace views {

namespace {

bool IsPopupWindow(HWND hwnd) {
  while (hwnd) {
    auto const dwStyle = static_cast<DWORD>(::GetWindowLong(hwnd, GWL_STYLE));
    if (dwStyle & WS_POPUP)
      return true;
    if (!(dwStyle & WS_CHILD))
      return false;
    hwnd = ::GetParent(hwnd);
  }
  return false;
}

}  // namespace

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

Frame* ContentWindow::GetFrame() const {
  for (auto runner = static_cast<const Widget*>(this); runner;
       runner = &runner->container_widget()) {
    if (runner->is<Frame>())
      return const_cast<Frame*>(runner->as<Frame>());
  }
  NOTREACHED();
  return nullptr;
}

ui::SelectionState ContentWindow::GetSelectionState() const {
  if (has_focus())
    return ui::SelectionState::HasFocus;

  if (!IsPopupWindow(::GetFocus()))
    return ui::SelectionState::Disabled;

  if (IsActive())
    return ui::SelectionState::Highlight;

  return ui::SelectionState::Disabled;
}

bool ContentWindow::IsActive() const {
  return GetFrame() == views::FrameList::instance()->active_frame();
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
