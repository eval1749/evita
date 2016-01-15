// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_Frame.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <windowsx.h>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/win/native_window.h"
#include "evita/css/style.h"
#include "evita/ctrl_TitleBar.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/gfx/dx_device.h"
#include "evita/gfx/direct2d_factory_win.h"
#include "evita/gfx/font.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/root_layer.h"
#include "evita/ui/events/event.h"
#include "evita/views/content_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/frame_observer.h"
#include "evita/views/message_view.h"
#include "evita/views/switches.h"
#include "evita/views/tabs/tab_strip.h"
#include "evita/views/tabs/tab_strip_animator.h"
#include "evita/layout/render_font_set.h"
#include "evita/views/window_set.h"
#include "evita/vi_EditPane.h"

using common::win::Rect;
using namespace views;  // NOLINT

namespace {

bool HasChildWindow(ui::Widget* parent, views::Window* window) {
  for (auto child : parent->child_nodes()) {
    if (child == window)
      return true;
  }
  return false;
}

TabContent* GetTabContentFromWindow(Frame* frame, views::Window* window) {
  for (auto child : frame->child_nodes()) {
    if (HasChildWindow(child, window))
      return child->as<TabContent>();
  }
  return nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Frame
//
Frame::Frame(views::WindowId window_id)
    : views::Window(ui::NativeWindow::Create(this), window_id),
      message_view_(new views::MessageView()),
      title_bar_(new views::TitleBar()),
      tab_strip_(new views::TabStrip(this)),
      tab_strip_animator_(new TabStripAnimator(tab_strip_)) {
  TabDataSet::instance()->AddObserver(this);
  AppendChild(tab_strip_);
  AppendChild(message_view_);
}

Frame::~Frame() {
  TabDataSet::instance()->RemoveObserver(this);
}

void Frame::AddObserver(views::FrameObserver* observer) {
  observers_.AddObserver(observer);
}

void Frame::AddTabContent(views::ContentWindow* content) {
  auto const tab_content = new EditPane();
  tab_content->AddObserver(this);
  tab_contents_.insert(tab_content);
  AppendChild(tab_content);
  if (!is_realized()) {
    DCHECK(!content->is_realized());
    tab_content->SetContent(content);
    return;
  }
  tab_content->SetBounds(GetTabContentBounds());
  tab_content->RealizeWidget();
  // |content| can be realized when it is dropped from another |Frame|.
  // So, we should set content after realize |TabContent|.
  tab_content->SetContent(content);
  tab_strip_animator_->AddTab(tab_content);
}

void Frame::AddOrActivateTabContent(views::ContentWindow* window) {
  DCHECK(!window->parent_node());
  DCHECK(!window->is_realized());
  if (auto const tab_content = GetRecentTabContent()) {
    if (auto const edit_tab_content = tab_content->as<EditPane>()) {
      if (edit_tab_content->has_more_than_one_child()) {
        edit_tab_content->ReplaceActiveContent(window);
        window->Activate();
        return;
      }
    }
  }

  AddTabContent(window);
  if (!window->is_realized())
    return;
  window->RequestFocus();
}

void Frame::DrawForResize() {
  auto const tab_content = tab_strip_animator_->active_tab_content();
  if (!tab_content)
    return;
  // TODO(eval1749): Is doing animation for active tab content here right thing?
  auto const now = base::TimeTicks::Now();
  for (auto child : tab_content->child_nodes()) {
    auto const window = child->as<Window>();
    if (!window)
      continue;
    window->HandleAnimationFrame(now);
  }
}

TabContent* Frame::GetRecentTabContent() {
  auto recent_tab_content = static_cast<TabContent*>(nullptr);
  for (auto tab_content : tab_contents_) {
    if (!recent_tab_content ||
        recent_tab_content->active_tick() < tab_content->active_tick()) {
      recent_tab_content = tab_content;
    }
  }

  return recent_tab_content;
}

Rect Frame::GetTabContentBounds() const {
  auto const message_view_height = message_view_->bounds().height();
  return gfx::Rect(bounds().left(), tab_strip_->bounds().bottom(),
                   bounds().right(), bounds().bottom() - message_view_height);
}

TabContent* Frame::GetTabContentByTabIndex(int const index) const {
  auto const present = tab_strip_->GetTab(index);
  if (!present)
    return nullptr;

  for (auto tab_content : tab_contents_) {
    if (tab_content == present)
      return present;
  }

  return nullptr;
}

int Frame::GetTabIndexOfTabContent(TabContent* const tab_content) const {
  auto const num_tabs = tab_strip_->number_of_tabs();
  for (auto tab_index = 0; tab_index < num_tabs; ++tab_index) {
    if (tab_strip_->GetTab(tab_index) == tab_content)
      return tab_index;
  }
  return -1;
}

void Frame::OnDropFiles(HDROP const drop_handle) {
  auto index = 0u;
  for (;;) {
    base::string16 file_name(MAX_PATH + 1, 0);
    auto const length = ::DragQueryFile(drop_handle, index, &file_name[0],
                                        static_cast<DWORD>(file_name.size()));
    if (!length)
      break;
    file_name.resize(length);
    editor::Application::instance()->view_event_handler()->OpenFile(window_id(),
                                                                    file_name);
    ++index;
  }
  ::DragFinish(drop_handle);
}

// Set status bar with formatted message on specified part.
void Frame::SetStatusBar(const std::vector<base::string16>& texts) {
  message_view_->SetStatus(texts);
}

void Frame::ShowMessage(MessageLevel, const base::string16& text) const {
  message_view_->SetMessage(text);
}

void Frame::UpdateTitleBar(const domapi::TabData& tab_data) {
  auto const window_title = base::StringPrintf(
      L"%ls %lc %ls", tab_data.title.c_str(),
      tab_data.state == domapi::TabData::State::Modified ? '*' : '-',
      editor::Application::instance()->title().c_str());
  title_bar_->SetText(window_title);
}

// ui::AnimationFrameHandler
void Frame::DidBeginAnimationFrame(const base::TimeTicks& now) {
  // Nothing to do
}

// ui::Widget
void Frame::CreateNativeWindow() const {
  int const cColumns = 83;
  int const cRows = 40;

  // Note: WS_EX_COMPOSITED posts WM_PAINT many times.
  // Note: WS_EX_LAYERED doesn't show window with Win7+.
  DWORD const dwExStyle =
      WS_EX_APPWINDOW | WS_EX_NOPARENTNOTIFY | WS_EX_WINDOWEDGE | WS_EX_LAYERED;
  DWORD const dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;

  const auto& font = *layout::FontSet::GetFont(*css::Style::Default(), 'x');
  gfx::SizeF size(font.GetCharWidth('M') * cColumns, font.height() * cRows);
  gfx::RectF rect(gfx::PointF(), gfx::Direct2DFactory::AlignToPixel(size));

  RECT raw_original_bounds(gfx::ToEnclosingRect(rect));
  ::AdjustWindowRectEx(&raw_original_bounds, dwStyle, TRUE, dwExStyle);
  Rect original_bounds(raw_original_bounds);

  auto const window_bounds = gfx::Rect(
      original_bounds.origin(),
      gfx::Size(original_bounds.width() + ::GetSystemMetrics(SM_CXVSCROLL) + 10,
                original_bounds.height()));

  RECT raw_workarea_bounds;
  ::SystemParametersInfo(SPI_GETWORKAREA, 0, &raw_workarea_bounds, 0);

  Rect workarea_bounds(raw_workarea_bounds);
  native_window()->CreateWindowEx(
      dwExStyle, dwStyle, L"", nullptr,
      gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      gfx::Size(window_bounds.width(), workarea_bounds.height() * 4 / 5));
  ::SetLayeredWindowAttributes(*native_window(), 0, 240, LWA_ALPHA);
}

void Frame::DidAddChildWidget(ui::Widget* new_child) {
  if (auto tab_content = new_child->as<TabContent>()) {
    tab_contents_.insert(tab_content);
    if (!is_realized()) {
      DCHECK(!new_child->is_realized());
      return;
    }
    tab_content->SetBounds(GetTabContentBounds());
    if (!tab_content->is_realized())
      tab_content->RealizeWidget();
    tab_strip_animator_->AddTab(tab_content);
    return;
  }

  auto const new_content = new_child->as<views::ContentWindow>();
  DCHECK(new_content);
  AddTabContent(new_content);
}

void Frame::DidChangeBounds() {
  views::Window::DidChangeBounds();
  const auto tab_content_bounds = GetTabContentBounds();

  tab_strip_->SetBounds(
      tab_strip_->bounds().origin(),
      gfx::Size(bounds().width(), tab_strip_->bounds().height()));

  message_view_->SetBounds(
      bounds().bottom_left() - gfx::Size(0, message_view_->bounds().height()),
      bounds().bottom_right());

  for (auto tab_content : tab_contents_)
    tab_content->SetBounds(tab_content_bounds);

  // Display resizing information.
  message_view_->SetMessage(base::StringPrintf(
      L"Resizing... %dx%d", bounds().width(), bounds().height()));

  DrawForResize();
}

void Frame::DidRealize() {
  views::FrameList::instance()->AddFrame(this);
  ::DragAcceptFiles(*native_window(), TRUE);

  title_bar_->Realize(*native_window());

  // TODO(eval1749): How do we determine height of TabStrip?
  auto const tab_strip_height =
      static_cast<Widget*>(tab_strip_)->GetPreferredSize().height();
  const auto close_button_height = ::GetSystemMetrics(SM_CYSIZE);
  tab_strip_->SetBounds(gfx::Point(0, close_button_height),
                        gfx::Size(bounds().width(), tab_strip_height));

  SetLayer(new ui::RootLayer(this));
  tab_content_layer_.reset(new ui::Layer());
  tab_strip_animator_->SetTabContentLayer(tab_content_layer_.get());

  auto const tab_content_bounds = GetTabContentBounds();
  for (auto tab_content : tab_contents_)
    tab_content->SetBounds(tab_content_bounds);

  // Place message vie at bottom of editor window.
  {
    auto const size =
        static_cast<ui::Widget*>(message_view_)->GetPreferredSize();
    message_view_->SetBounds(
        bounds().bottom_left() - gfx::Size(0, size.height()),
        bounds().bottom_right());
  }

  // Realize message view, tab_contents and tab strip.
  views::Window::DidRealize();
  layer()->AppendLayer(tab_strip_->layer());
  layer()->AppendLayer(tab_content_layer_.get());
  layer()->AppendLayer(message_view_->layer());
  for (auto tab_content : tab_contents_)
    tab_strip_animator_->AddTab(tab_content);

  ::SetWindowPos(AssociatedHwnd(), nullptr, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER |
                     SWP_NOREDRAW | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void Frame::DidRemoveChildWidget(ui::Widget* old_child) {
  views::Window::DidRemoveChildWidget(old_child);
  if (!tab_contents_.empty())
    return;
  DestroyWidget();
}

void Frame::DidRequestDestroy() {
  editor::Application::instance()->view_event_handler()->QueryClose(
      window_id());
}

void Frame::DidSetFocus(ui::Widget* widget) {
  views::Window::DidSetFocus(widget);
  FOR_EACH_OBSERVER(views::FrameObserver, observers_, DidActivateFrame(this));
  if (!tab_strip_animator_->active_tab_content()) {
    // This |Frame| doesn't show anything and tabs are being added.
    return;
  }
  auto const tab_content = GetRecentTabContent();
  if (!tab_content)
    return;
  tab_content->RequestFocus();
}

LRESULT Frame::OnMessage(uint32_t message,
                         WPARAM const wParam,
                         LPARAM const lParam) {
  switch (message) {
    case WM_ACTIVATE: {
      MARGINS margins;
      margins.cxLeftWidth = 0;
      margins.cxRightWidth = 0;
      margins.cyBottomHeight = 0;
      margins.cyTopHeight = -1;
      COM_VERIFY(::DwmExtendFrameIntoClientArea(*native_window(), &margins));
      break;
    }

    case WM_DROPFILES:
      OnDropFiles(reinterpret_cast<HDROP>(wParam));
      break;

    case WM_ENTERSIZEMOVE:
      for (auto tab_content : tab_contents_)
        tab_content->DidEnterSizeMove();
      break;

    case WM_EXITSIZEMOVE:
      for (auto tab_content : tab_contents_)
        tab_content->DidExitSizeMove();
      message_view_->SetMessage(base::StringPrintf(
          L"Resized to %dx%d", bounds().width(), bounds().height()));
      break;

    case WM_GETMINMAXINFO: {
      auto pMinMax = reinterpret_cast<MINMAXINFO*>(lParam);
      pMinMax->ptMinTrackSize.x = 200;
      pMinMax->ptMinTrackSize.y = 200;
      return 0;
    }

    case WM_NCCALCSIZE: {
      if (tab_strip_->bounds().empty())
        break;
      if (!wParam)
        return 0;
      // Remove icon and title from caption.
      auto const params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
      RECT* client_rect = &params->rgrc[0];
      // Compute edge from proposed window area and client area, because
      // GetSystemMetrics(SM_CXSIZEFRAME) returns 4, but proposed edge size
      // is 8.
      const auto kEdgeSize = params->rgrc[2].left - params->rgrc[1].left;
      client_rect->left += kEdgeSize;
      client_rect->right -= kEdgeSize;
      client_rect->bottom -= kEdgeSize;
      // Note: We should not return WVR_REDRAW to avoid resize flicker.
      return 0;
    }

    case WM_NCHITTEST: {
      LRESULT lResult;
      if (::DwmDefWindowProc(*native_window(), message, wParam, lParam,
                             &lResult)) {
        return lResult;
      }
      gfx::Point desktop_point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      auto const point = MapFromDesktopPoint(desktop_point);
      if (point.y() < 4)
        return HTTOP;
      if (!tab_strip_->bounds().empty()) {
        auto const result = tab_strip_->NonClientHitTest(desktop_point);
        if (result != HTNOWHERE)
          return result;
      }

      // Resize button in message view
      if (message_view_) {
        auto const button_size = ::GetSystemMetrics(SM_CXVSCROLL);
        gfx::Rect resize_button(
            bounds().bottom_right() - gfx::Size(button_size, button_size),
            bounds().bottom_right());
        if (resize_button.Contains(point))
          return HTBOTTOMRIGHT;
      }
      break;
    }

    case WM_NOTIFY:
      if (!tab_strip_)
        break;
      return tab_strip_->OnNotify(reinterpret_cast<NMHDR*>(lParam));
  }

  return ui::Widget::OnMessage(message, wParam, lParam);
}

void Frame::OnPaint(const gfx::Rect) {
  // Fill window client area with alpha=0.
  PAINTSTRUCT ps;
  auto const hdc = ::BeginPaint(AssociatedHwnd(), &ps);
  ::FillRect(hdc, &ps.rcPaint,
             static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)));
  ::EndPaint(AssociatedHwnd(), &ps);
}

void Frame::WillDestroyWidget() {
  views::Window::WillDestroyWidget();
  tab_content_layer_.reset();
  views::FrameList::instance()->RemoveFrame(this);
}

void Frame::WillRemoveChildWidget(ui::Widget* old_child) {
  views::Window::WillRemoveChildWidget(old_child);
  if (message_view_ == old_child) {
    message_view_ = nullptr;
    return;
  }
  if (tab_strip_ == old_child) {
    tab_strip_ = nullptr;
    return;
  }
  if (!is_realized() || !tab_strip_)
    return;
  auto const tab_content = old_child->as<TabContent>();
  if (!tab_content)
    return;

  tab_strip_animator_->DidDeleteTabContent(tab_content);

  DCHECK(tab_contents_.find(tab_content) != tab_contents_.end());
  tab_contents_.erase(tab_content);

  auto const tab_index = GetTabIndexOfTabContent(tab_content);
  DCHECK_GE(tab_index, 0);
  tab_strip_->DeleteTab(tab_index);
}

// views::TabContentObserver
void Frame::DidActivateTabContent(TabContent* tab_content) {
  tab_strip_animator_->RequestSelect(tab_content);
}

// views::TabDataSet::Observer
void Frame::DidSetTabData(domapi::WindowId window_id,
                          const domapi::TabData& tab_data) {
  auto const window = Window::FromWindowId(window_id);
  if (!window)
    return;
  auto const content_window = window->as<ContentWindow>();
  if (!content_window || !content_window->parent_node())
    return;
  auto const tab_content = content_window->parent_node()->as<TabContent>();
  auto const tab_index = GetTabIndexOfTabContent(tab_content);
  if (tab_index < 0)
    return;
  tab_strip_->SetTab(tab_index, tab_data);
  auto const active_tab_content = tab_strip_animator_->active_tab_content();
  if (active_tab_content && tab_content != active_tab_content)
    return;
  UpdateTitleBar(tab_data);
}

// views::TabStripDelegate
void Frame::DidDropTab(TabContent* tab_content,
                       const gfx::Point& screen_point) {
  struct Local {
    static Frame* GetFrameFromPoint(const gfx::Point& screen_point) {
      for (auto hwnd = ::WindowFromPoint(screen_point); hwnd;
           hwnd = ::GetParent(hwnd)) {
        if (auto const frame = FrameList::instance()->FindFrameByHwnd(hwnd)) {
          return frame;
        }
      }
      return nullptr;
    }
  };

  auto const edit_tab_content = tab_content->as<EditPane>();
  if (!edit_tab_content)
    return;

  auto const frame = Local::GetFrameFromPoint(screen_point);
  if (frame == this)
    return;

  if (!frame) {
    editor::Application::instance()->view_event_handler()->DidDropWidget(
        edit_tab_content->GetActiveContent()->window_id(),
        views::kInvalidWindowId);
    return;
  }

  DCHECK_NE(frame, tab_content->parent_node());
  editor::Application::instance()->view_event_handler()->DidDropWidget(
      edit_tab_content->GetActiveContent()->window_id(), frame->window_id());
}

void Frame::DidSelectTab(int selected_index) {
  if (selected_index < 0) {
    if (auto const recent_tab_content = GetRecentTabContent())
      tab_strip_animator_->RequestSelect(recent_tab_content);
    return;
  }
  auto const tab_content = GetTabContentByTabIndex(selected_index);
  if (!tab_content || !has_native_focus())
    return;
  tab_content->RequestFocus();
  const auto tab_data = tab_content->GetTabData();
  if (!tab_data)
    return;
  UpdateTitleBar(*tab_data);
}

void Frame::RequestCloseTab(int tab_index) {
  if (tab_contents_.size() == 1u) {
    // If this window has only one tab_content, we destroy this window.
    editor::Application::instance()->view_event_handler()->QueryClose(
        window_id());
    return;
  }

  auto const tab_content = GetTabContentByTabIndex(tab_index);
  DCHECK(tab_content);
  tab_content->DestroyWidget();
}

void Frame::RequestSelectTab(int selected_index) {
  auto const tab_content = GetTabContentByTabIndex(selected_index);
  if (!tab_content)
    return;
  tab_strip_animator_->RequestSelect(tab_content);
}
