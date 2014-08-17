// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_Frame.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "common/tree/ancestors_or_self.h"
#include "common/tree/child_nodes.h"
#include "common/win/native_window.h"
#include "evita/css/style.h"
#include "evita/ctrl_TitleBar.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/gfx_base.h"
#include "evita/gfx/dx_device.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/root_layer.h"
#include "evita/ui/events/event.h"
#include "evita/views/content_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/frame_observer.h"
#include "evita/views/icon_cache.h"
#include "evita/views/message_view.h"
#include "evita/views/switches.h"
#include "evita/views/tab_strip.h"
#include "evita/views/tab_strip_animator.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/window_set.h"
#include "evita/vi_EditPane.h"

using common::win::Rect;
using namespace views;

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

} // namespace

//////////////////////////////////////////////////////////////////////
//
// Frame
//
Frame::Frame(views::WindowId window_id)
    : views::Window(ui::NativeWindow::Create(this), window_id),
      active_tab_content_(nullptr),
      message_view_(new views::MessageView()),
      title_bar_(new views::TitleBar()),
      tab_strip_(new views::TabStrip(this)),
      tab_strip_animator_(new TabStripAnimator(tab_strip_)) {
  AppendChild(tab_strip_);
  AppendChild(message_view_);
}

Frame::~Frame() {
}

bool Frame::Activate() {
  return ::SetForegroundWindow(*native_window());
}

void Frame::AddObserver(views::FrameObserver* observer) {
  observers_.AddObserver(observer);
}

void Frame::AddTabContent(views::ContentWindow* window) {
  auto const tab_content = new EditPane();
  tab_contents_.insert(tab_content);
  tab_content->SetContent(window);
  AppendChild(tab_content);
  if (!is_realized()) {
    DCHECK(!window->is_realized());
    return;
  }
  tab_content->SetBounds(GetTabContentBounds());
  tab_strip_animator_->AddTab(tab_content);
  tab_content->RealizeWidget();
}

void Frame::AddOrActivateTabContent(views::ContentWindow* window) {
  DCHECK(!window->parent_node());
  DCHECK(!window->is_realized());
  if (auto const tab_content = GetActiveTabContent()) {
    if (auto const edit_tab_content = tab_content->as<EditPane>()) {
      if (edit_tab_content->has_more_than_one_child()) {
        edit_tab_content->ReplaceActiveWindow(window);
        window->Activate();
        return;
      }
    }
  }

  AddTabContent(window);
}

void Frame::DidSetFocusOnChild(views::Window* window) {
  auto const tab_content = GetTabContentFromWindow(this, window);
  DCHECK(tab_content);

  if (tab_content == active_tab_content_) {
    DCHECK(window->visible());
    return;
  }

  DCHECK(!tab_content->visible());
  DCHECK(!window->visible());
  tab_strip_animator_->RequestSelect(tab_content);
}

void Frame::DrawForResize() {
  if (!active_tab_content_)
    return;
  auto const now = base::Time::Now();
  for (auto child : active_tab_content_->child_nodes()) {
    auto const window = child->as<Window>();
    if (!window)
      continue;
    ui::Animator::instance()->PlayAnimation(now, window);
  }
}

TabContent* Frame::GetActiveTabContent() {
  if (active_tab_content_ && active_tab_content_->active_tick())
    return active_tab_content_;

  auto active_tab_content = static_cast<TabContent*>(nullptr);
  for (auto tab_content : tab_contents_) {
    if (!active_tab_content ||
        active_tab_content->active_tick() < tab_content->active_tick()) {
      active_tab_content = tab_content;
    }
  }

  return active_tab_content;
}

static TabContent* GetTabContentAt(views::TabStrip* tab_strip, int const index) {
  TCITEM tab_item;
  tab_item.mask = TCIF_PARAM;
  if (!tab_strip->GetTab(index, &tab_item))
      return nullptr;
  return reinterpret_cast<TabContent*>(tab_item.lParam);
}

Rect Frame::GetTabContentBounds() const {
  auto const message_view_height = message_view_->bounds().height();
  return gfx::Rect(bounds().left(), tab_strip_->bounds().bottom(),
                   bounds().right(),
                   bounds().bottom() - message_view_height);
}

TabContent* Frame::GetTabContentByTabIndex(int const index) const {
  auto const present = GetTabContentAt(tab_strip_, index);
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
    if (GetTabContentAt(tab_strip_, tab_index) == tab_content)
      return tab_index;
  }
  return -1;
}

void Frame::OnDropFiles(HDROP const drop_handle) {
  auto index = 0u;
  for (;;) {
    base::string16 file_name(MAX_PATH + 1, 0);
    auto const length = ::DragQueryFile(drop_handle, index, &file_name[0],
                                        file_name.size());
    if (!length)
      break;
    file_name.resize(length);
    Application::instance()->view_event_handler()->OpenFile(
        window_id(), file_name);
    ++index;
  }
  ::DragFinish(drop_handle);
}

/// <summary>
///   Set status bar formatted message on specified part.
/// </summary>
void Frame::SetStatusBar(const std::vector<base::string16> texts) {
  message_view_->SetStatus(texts);
}

void Frame::ShowMessage(MessageLevel, const base::string16& text) const {
  message_view_->SetMessage(text);
}

// [U]
/// <summary>
///   Updates title bar to display active buffer.
/// </summary>
void Frame::UpdateTitleBar() {
  if (!active_tab_content_)
    return;
  auto const tab_data = active_tab_content_->GetTabData();
  if (!tab_data)
    return;
  auto& title = tab_data->title;
  auto const tab_index = GetTabIndexOfTabContent(active_tab_content_);
  if (tab_index >= 0) {
    TCITEM tab_item = {0};
    tab_item.mask = TCIF_IMAGE | TCIF_STATE | TCIF_TEXT;
    tab_item.pszText = const_cast<LPWSTR>(title.c_str());
    tab_item.iImage = tab_data->icon;
    tab_item.dwState = static_cast<DWORD>(
        tab_data->state == domapi::TabData::State::Modified);
    tab_item.dwStateMask = 1;
    // TODO(yosi) We should not use magic value -2 for tab_data->icon.
    if (tab_item.iImage == -2) {
      tab_item.iImage = views::IconCache::instance()->GetIconForFileName(
          tab_data->title);
    }
    tab_item.iImage = std::max(tab_item.iImage, 0);
    tab_strip_->SetTab(tab_index, &tab_item);
  }

  auto const window_title = title +
      (tab_data->state == domapi::TabData::State::Modified ? L" * " : L" - ") +
      Application::instance()->title();
  title_bar_->SetText(window_title);
}

// ui::Animatable
void Frame::Animate(base::Time now) {
  if (!visible())
    return;
  ui::Animator::instance()->ScheduleAnimation(this);
  // TODO(eval1749) We should call update title bar when needed.
  UpdateTitleBar();

  DEFINE_STATIC_LOCAL(base::Time, busy_start_at, ());
  static bool busy;
  {
    UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
    if (lock_scope.locked()) {
      busy = false;
      return;
    }
  }

  if (!busy) {
    busy = true;
    busy_start_at = now;
    return;
  }

  auto const delta = (now - busy_start_at).InSeconds();
  if (!delta)
    return;

  message_view_->SetMessage(base::StringPrintf(
    L"Script runs %ds. Ctrl+Break to terminate script.",
    delta));
}

// ui::Widget
void Frame::CreateNativeWindow() const {
  int const cColumns = 83;
  int const cRows    = 40;

  // Note: WS_EX_COMPOSITED posts WM_PAINT many times.
  // Note: WS_EX_LAYERED doesn't show window with Win7+.
  DWORD dwExStyle =
      WS_EX_APPWINDOW
      | WS_EX_NOPARENTNOTIFY
      | WS_EX_WINDOWEDGE;

  DWORD dwStyle =
    WS_OVERLAPPEDWINDOW
    | WS_CLIPCHILDREN
    | WS_VISIBLE;

  const auto& font = *views::rendering::FontSet::GetFont(*css::Style::Default(),
                                                         'x');
  gfx::SizeF size(font.GetCharWidth('M') * cColumns,
                  font.height() * cRows);
  gfx::RectF rect(gfx::PointF(), gfx::FactorySet::AlignToPixel(size));

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
}

void Frame::DidAddChildWidget(const ui::Widget& widget) {
  if (auto tab_content = const_cast<TabContent*>(widget.as<TabContent>())) {
    tab_contents_.insert(tab_content);
    if (!is_realized()) {
      DCHECK(!widget.is_realized());
      return;
    }
    if (tab_content->is_realized())
      tab_content->SetBounds(GetTabContentBounds());
    else
      tab_content->Realize(GetTabContentBounds());
    tab_strip_animator_->AddTab(tab_content);
    return;
  }

  auto window = const_cast<views::ContentWindow*>(
      widget.as<views::ContentWindow>());
  DCHECK(window);
  AddTabContent(window);
}

void Frame::DidChangeBounds() {
  views::Window::DidChangeBounds();
  const auto tab_content_bounds = GetTabContentBounds();

  tab_strip_->SetBounds(gfx::Rect(
      tab_strip_->bounds().origin(),
      gfx::Size(bounds().width(), tab_strip_->bounds().height())));

  message_view_->SetBounds(gfx::Rect(
    bounds().bottom_left() - gfx::Size(0, message_view_->bounds().height()),
    bounds().bottom_right()));

  for (auto tab_content : tab_contents_) {
    tab_content->SetBounds(tab_content_bounds);
  }

  // Display resizing information.
  message_view_->SetMessage(base::StringPrintf(L"Resizing... %dx%d",
      bounds().width(), bounds().height()));

  DrawForResize();
  ui::Compositor::instance()->CommitIfNeeded();
}

void Frame::DidRealize() {
  views::FrameList::instance()->AddFrame(this);
  ::DragAcceptFiles(*native_window(), TRUE);

  title_bar_->Realize(*native_window());

  // TODO(yosi) How do we determine height of TabStrip?
  auto const tab_strip_height = static_cast<Widget*>(tab_strip_)->
      GetPreferredSize().height();
  const auto close_button_height = ::GetSystemMetrics(SM_CYSIZE);
  tab_strip_->SetBounds(Rect(0, close_button_height,
                             bounds().width(),
                             close_button_height + 4 + tab_strip_height));

  SetLayer(new ui::RootLayer(this));
  tab_content_layer_.reset(new ui::Layer());
  tab_strip_animator_->SetLayer(tab_content_layer_.get());

  auto const tab_content_bounds = GetTabContentBounds();
  for (auto tab_content : tab_contents_) {
    tab_content->SetBounds(tab_content_bounds);
  }

  // Place message vie at bottom of editor window.
  {
    auto const size = static_cast<ui::Widget*>(message_view_)->
        GetPreferredSize();
    message_view_->SetBounds(gfx::Rect(
        bounds().bottom_left() - gfx::Size(0, size.height()),
        bounds().bottom_right()));
  }

  // Create message view, tab_contents and tab strip.
  views::Window::DidRealize();
  layer()->AppendChildLayer(tab_content_layer_.get());
  layer()->AppendChildLayer(message_view_->layer());
  for (auto tab_content : tab_contents_) {
    tab_strip_animator_->AddTab(tab_content);
  }

  ::SetWindowPos(AssociatedHwnd(), nullptr, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER |
                 SWP_NOREDRAW | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void Frame::DidRemoveChildWidget(const ui::Widget& widget) {
  views::Window::DidRemoveChildWidget(widget);
  auto const tab_content = const_cast<TabContent*>(widget.as<TabContent>());
  if (!tab_content)
    return;
  DCHECK(tab_contents_.find(tab_content)!= tab_contents_.end());
  tab_contents_.erase(tab_content);
  if (first_child())
    return;
  DCHECK(tab_contents_.empty());
  DestroyWidget();
}

void Frame::DidSetFocus(ui::Widget* widget) {
  views::Window::DidSetFocus(widget);
  if (!active_tab_content_) {
    active_tab_content_ = GetActiveTabContent();
    if (!active_tab_content_)
      return;
  }
  active_tab_content_->RequestFocus();
  FOR_EACH_OBSERVER(views::FrameObserver, observers_, DidActiveFrame(this));
}

LRESULT Frame::OnMessage(uint32_t message, WPARAM const wParam,
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

    case WM_CLOSE:
      Application::instance()->view_event_handler()->QueryClose(window_id());
      return 0;

    case WM_DROPFILES:
      OnDropFiles(reinterpret_cast<HDROP>(wParam));
      break;

    case WM_EXITSIZEMOVE:
      message_view_->SetMessage(base::StringPrintf(L"Resized to %dx%d",
          bounds().width(), bounds().height()));
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
      if (tab_strip_ && point.y() < tab_strip_->bounds().bottom())
        return HTCAPTION;

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
  views::FrameList::instance()->RemoveFrame(this);
}

void Frame::WillRemoveChildWidget(const ui::Widget& widget) {
  views::Window::WillRemoveChildWidget(widget);
  if (!is_realized())
    return;
  auto const tab_content = const_cast<TabContent*>(widget.as<TabContent>());
  if (!tab_content)
    return;
  auto const tab_index = GetTabIndexOfTabContent(tab_content);
  DCHECK_GE(tab_index, 0);
  tab_strip_->DeleteTab(tab_index);
}

// views::TabStripDelegate
void Frame::DidClickTabCloseButton(int tab_index) {
  if (tab_contents_.size() == 1u) {
    // If this window has only one tab_content, we destroy this window.
    Application::instance()->view_event_handler()->QueryClose(
        window_id());
    return;
  }

  auto const tab_content = GetTabContentByTabIndex(tab_index);
  DCHECK(tab_content);
  tab_content->DestroyWidget();
}

void Frame::DidChangeTabSelection(int selected_index) {
  auto const tab_content = GetTabContentByTabIndex(selected_index);
  if (!tab_content ||active_tab_content_ == tab_content)
    return;
  active_tab_content_ = tab_content;
  active_tab_content_->RequestFocus();
}

void Frame::DidThrowTab(LPARAM lParam) {
  auto const tab_content = reinterpret_cast<TabContent*>(lParam);
  auto const edit_tab_content = tab_content->as<EditPane>();
  if (!edit_tab_content)
    return;
  Application::instance()->view_event_handler()->DidDropWidget(
      edit_tab_content->GetActiveWindow()->window_id(),
      views::kInvalidWindowId);
}

base::string16 Frame::GetTooltipTextForTab(int tab_index) {
  auto const tab_content = GetTabContentByTabIndex(static_cast<int>(tab_index));
  if (!tab_content)
    return base::string16();
  auto const tab_data = tab_content->GetTabData();
  if (!tab_data)
    return base::string16();
  return tab_data->tooltip;
}

void Frame::OnDropTab(LPARAM lParam) {
  auto const tab_content = reinterpret_cast<TabContent*>(lParam);
  if (tab_content->parent_node() == this)
    return;
  auto const edit_tab_content = tab_content->as<EditPane>();
  if (!edit_tab_content)
    return;
  Application::instance()->view_event_handler()->DidDropWidget(
      edit_tab_content->GetActiveWindow()->window_id(),
      window_id());
}

void Frame::RequestSelectTab(int selected_index) {
  auto const tab_content = GetTabContentByTabIndex(selected_index);
  if (!tab_content)
    return;
  tab_strip_animator_->RequestSelect(tab_content);
}
