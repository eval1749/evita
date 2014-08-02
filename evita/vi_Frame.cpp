// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DROPFILES 0
#define DEBUG_FOCUS     0
#define DEBUG_PAINT     0
#define DEBUG_REDRAW    0
#define DEBUG_WINDOWPOS 0
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
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/root_layer.h"
#include "evita/ui/events/event.h"
#include "evita/views/content_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/frame_observer.h"
#include "evita/views/icon_cache.h"
#include "evita/views/message_view.h"
#include "evita/views/metrics_view.h"
#include "evita/views/switches.h"
#include "evita/views/tab_data_set.h"
#include "evita/views/tab_strip.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/window_set.h"
#include "evita/vi_EditPane.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;

using common::win::Rect;
using namespace views;

namespace {

class CompositionState {
  private: BOOL enabled_;
  private: CompositionState() : enabled_(false) {}

  private: static CompositionState* instance() {
    static CompositionState* instance;
    if (!instance)
      instance = new CompositionState();
    return instance;
  }

  public: static bool IsEnabled() { return instance()->enabled_; }

  public: static void Update() {
    HRESULT hr = ::DwmIsCompositionEnabled(&instance()->enabled_);
    if (FAILED(hr))
          instance()->enabled_ = false;
  }

  public: static void Update(HWND hwnd) {
    ASSERT(hwnd);
    Update();
// When USE_LAYERED is true, background of tab band doesn't have glass effect.
#define USE_LAYERED 0
#if USE_LAYERED
    if (IsEnabled()) {
      ::SetWindowLong(hwnd, GWL_EXSTYLE,
          ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
      //::SetLayeredWindowAttributes(hwnd, 0, (255 * 70) / 100, LWA_ALPHA);
      ::SetLayeredWindowAttributes(
          hwnd,
          RGB(255, 0, 0), (255 * 10) / 100, LWA_COLORKEY);
    } else {
      ::SetWindowLong(hwnd, GWL_EXSTYLE,
          ::GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
      ::RedrawWindow(hwnd, nullptr, nullptr,
          RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }
#endif
  }
};

bool HasChildWindow(ui::Widget* parent, views::Window* window) {
  for (auto child : parent->child_nodes()) {
    if (child == window)
      return true;
  }
  return false;
}

Pane* GetContainingPane(Frame* frame, views::Window* window) {
  for (auto child : frame->child_nodes()) {
    if (HasChildWindow(child, window))
      return child->as<Pane>();
  }
  return nullptr;
}

} // namespace

//////////////////////////////////////////////////////////////////////
//
// Frame::DragController
//
class Frame::DragController final {
  private: bool drag_in_progress_;
  private: gfx::Size drag_offset_;
  private: Frame* frame_;

  public: DragController(Frame* frame);
  public: ~DragController() = default;

  public: void OnMouseMoved(const ui::MouseEvent& event);
  public: void OnMousePressed(const ui::MouseEvent& event);
  public: void OnMouseReleased(const ui::MouseEvent& event);

  DISALLOW_COPY_AND_ASSIGN(DragController);
};

Frame::DragController::DragController(Frame* frame)
    : drag_in_progress_(false), frame_(frame) {
}

void Frame::DragController:: OnMouseMoved(const ui::MouseEvent& event) {
  if (!drag_in_progress_)
    return;
  auto const view = frame_->metrics_view_;
  view->SetBounds(gfx::Rect(event.location() - drag_offset_,
                            view->bounds().size()));
  ui::Compositor::instance()->CommitIfNeeded();
  ui::Compositor::instance()->WaitForCommitCompletion();
}

void Frame::DragController:: OnMousePressed(const ui::MouseEvent& event) {
  auto const view = frame_->metrics_view_;
  if (!view->bounds().Contains(event.location()))
    return;
  drag_in_progress_ = true;
  drag_offset_ = event.location() - view->bounds().origin();
  frame_->SetCapture();
}

void Frame::DragController:: OnMouseReleased(const ui::MouseEvent&) {
  if (!drag_in_progress_)
    return;
  drag_in_progress_ = false;
  frame_->ReleaseCapture();
}

//////////////////////////////////////////////////////////////////////
//
// Frame
//
Frame::Frame(views::WindowId window_id)
    : views::Window(ui::NativeWindow::Create(this), window_id),
      drag_controller_(new DragController(this)),
      message_view_(new views::MessageView()),
      metrics_view_(new views::MetricsView()),
      title_bar_(new views::TitleBar()),
      tab_strip_(new views::TabStrip(this)),
      m_pActivePane(nullptr) {
  AppendChild(tab_strip_);
  AppendChild(message_view_);
  AppendChild(metrics_view_);
}

Frame::~Frame() {
}

Frame::operator HWND() const {
  return *native_window();
}

bool Frame::Activate() {
  return ::SetForegroundWindow(*native_window());
}

void Frame::AddObserver(views::FrameObserver* observer) {
  observers_.AddObserver(observer);
}

void Frame::AddPane(views::ContentWindow* window) {
  auto const pane = new EditPane();
  m_oPanes.Append(this, pane);
  AppendChild(pane);
  // TODO(eval1749) We should have more sophisticated way to keep top most
  // window.
  AppendChild(metrics_view_);
  if (!is_realized()) {
    DCHECK(!window->is_realized());
    pane->SetContent(window);
    return;
  }
  pane->Realize(GetPaneRect());
  pane->SetContent(window);
  AddTab(pane);
}

void Frame::AddTab(Pane* const pane) {
  ASSERT(is_realized());
  ASSERT(pane->is_realized());
  // Set dummy tab label. Actual tab label will be set later in
  // |Frame::updateTitleBar|.
  TCITEM tab_item;
  tab_item.mask = TCIF_IMAGE| TCIF_TEXT | TCIF_PARAM;
  tab_item.pszText = L"?";
  tab_item.lParam = reinterpret_cast<LPARAM>(pane);
  tab_item.iImage = 0;
  auto const new_tab_item_index = tab_strip_->number_of_tabs();
  tab_strip_->InsertTab(new_tab_item_index, &tab_item);
  tab_strip_->SelectTab(new_tab_item_index);
}

void Frame::AddWindow(views::ContentWindow* window) {
  DCHECK(!window->parent_node());
  DCHECK(!window->is_realized());
  if (auto const pane = GetActivePane()) {
    if (auto const edit_pane = pane->as<EditPane>()) {
      if (edit_pane->has_more_than_one_child()) {
        edit_pane->ReplaceActiveWindow(window);
        window->Activate();
        return;
      }
    }
  }

  AddPane(window);
}

void Frame::DidActivatePane(Pane* const pane) {
  auto const tab_index = getTabFromPane(pane);
  if (tab_index < 0)
    return;
  auto const selected_index = tab_strip_->selected_index();
  #if DEBUG_FOCUS
   DVLOG(0) << "DidActivatePane selected_index=" << selected_index <<
      " cur=" << m_pActivePane << " .focus=" << m_pActivePane->has_focus <<
      " new=" << pane << " .focus=" << pane->has_focus() <<
      " tab_index=" << tab_index;
  #endif

  if (tab_index != selected_index)
    tab_strip_->SelectTab(tab_index);
}

void Frame::DidAddChildWidget(const ui::Widget& widget) {
  if (auto pane = const_cast<Pane*>(widget.as<Pane>())) {
    m_oPanes.Append(this, pane);
    if (!is_realized()) {
      DCHECK(!widget.is_realized());
      return;
    }
    if (pane->is_realized())
      pane->SetBounds(GetPaneRect());
    else
      pane->Realize(GetPaneRect());
    AddTab(pane);
    return;
  }

  auto window = const_cast<views::ContentWindow*>(
      widget.as<views::ContentWindow>());
  DCHECK(window);
  AddPane(window);
}

void Frame::DidChangeTabSelection(int selected_index) {
  auto const pane = getPaneFromTab(selected_index);
  #if DEBUG_FOCUS
    DVLOG(0) << "DidChangeTabSelection Start"
        " selected_index=" << selected_index <<
        " cur=" << m_pActivePane << " new=" << pane;
  #endif
  if (!pane) {
    #if DEBUG_FOCUS
      DVLOG(0) << "selected_index(" << selected_index << " doesn't have pane!";
    #endif
    return;
  }
  if (m_pActivePane == pane) {
    #if DEBUG_FOCUS
      DVLOG(0) << "Active pane(" << selected_index << ") isn't changed. why?";
    #endif
    return;
  }
  if (m_pActivePane) {
    m_pActivePane->Hide();
  } else {
    #if DEBUG_FOCUS
      DVLOG(0) << "Why we don't have acitve pane?";
    #endif
  }
  m_pActivePane = pane;
  pane->Show();
  pane->Activate();
  #if DEBUG_FOCUS
    DVLOG(0) << "End selected_index=" << selected_index <<
        " cur=" m_pActivePane << " new=" << pane;
  #endif
}

void Frame::DidSetFocusOnChild(views::Window* window) {
  auto const pane = GetContainingPane(this, window);
  if (!pane) {
    DVLOG(0) << "Frame::DidSetFolcusOnChild: No pane contains " << window;
    return;
  }

  pane->UpdateActiveTick();

  if (window->is_shown())
    return;

  if (pane == m_pActivePane) {
    // TODO(yosi) This is happened on multiple window in one pane, we should
    // update tab label text.
    if (!window->is_shown())
      window->Show();
    return;
  }

  DCHECK(!window->is_shown());
  auto const tab_index = getTabFromPane(pane);
  if (tab_index >= 0)
    DidChangeTabSelection(tab_index);
}

// Note: We should call |ID2D1RenderTarget::Clear()| to reset alpha component
// of pixels.
void Frame::DrawForResize() {
#if 0
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  auto const pane_size = GetPaneRect().size();
  canvas_->set_dirty_rect(gfx::RectF(gfx::Point(), gfx::SizeF(
      static_cast<float>(pane_size.cx),
      static_cast<float>(pane_size.cy))));

  // To avoid script destroys Pane's, we lock DOM.
  if (editor::DomLock::instance()->locked()) {
    canvas_->Clear(gfx::ColorF(gfx::ColorF::White));
    for (auto& pane: m_oPanes) {
      pane.OnDraw(&*canvas_);
    }
    return;
  }

  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (lock_scope.locked()) {
    canvas_->Clear(gfx::ColorF(gfx::ColorF::White));
    for (auto& pane: m_oPanes) {
      pane.OnDraw(&*canvas_);
    }
    return;
  }
  canvas_->Clear(gfx::ColorF(gfx::ColorF::LightGray));
#endif
}

Frame* Frame::FindFrame(const ui::Widget& widget) {
  for (auto ancestor : common::tree::ancestors_or_self(&widget)) {
    if (ancestor->is<Frame>())
      return const_cast<Widget*>(ancestor)->as<Frame>();
  }
  return nullptr;
}

Pane* Frame::GetActivePane() {
  if (m_pActivePane && m_pActivePane->active_tick())
    return m_pActivePane;

  auto pActive = m_oPanes.GetFirst();
  for (auto& pane: m_oPanes) {
    if (pActive->active_tick() < pane.active_tick())
      pActive = &pane;
  }

  return pActive;
}

static Pane* getPaneAt(views::TabStrip* tab_strip, int const index) {
  TCITEM tab_item;
  tab_item.mask = TCIF_PARAM;
  if (!tab_strip->GetTab(index, &tab_item))
      return nullptr;
  return reinterpret_cast<Pane*>(tab_item.lParam);
}

Pane* Frame::getPaneFromTab(int const index) const {
  auto const present = getPaneAt(tab_strip_, index);
  if (!present)
    return nullptr;

  for (auto& pane: m_oPanes) {
    if (pane == present)
      return present;
  }

  return nullptr;
}

int Frame::getTabFromPane(Pane* const pane) const {
  auto const num_tabs = tab_strip_->number_of_tabs();
  for (auto tab_index = 0; tab_index < num_tabs; ++tab_index) {
    if (getPaneAt(tab_strip_, tab_index) == pane)
      return tab_index;
  }
  return -1;
}

Rect Frame::GetPaneRect() const {
  auto const message_view_height = ::GetSystemMetrics(SM_CYCAPTION);
  return gfx::Rect(bounds().left(), tab_strip_->bounds().bottom(),
                   bounds().right(),
                   bounds().bottom() - message_view_height);
}

void Frame::onDropFiles(HDROP const hDrop) {
  uint nIndex = 0;
  for (;;) {
    base::string16 file_name(MAX_PATH + 1, 0);
    auto const length = ::DragQueryFile(hDrop, nIndex, &file_name[0],
                                        file_name.size());
    if (!length)
      break;
    file_name.resize(length);
    Application::instance()->view_event_handler()->OpenFile(
        window_id(), file_name);
    nIndex += 1;
  }
  ::DragFinish(hDrop);
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
void Frame::updateTitleBar() {
  if (!m_pActivePane)
    return;
  auto const window = m_pActivePane->GetWindow();
  if (!window)
    return;
  auto const tab_data = TabDataSet::instance()->GetTabData(
      window->window_id());
  if (!tab_data)
    return;
  auto& title = tab_data->title;
  auto const tab_index = getTabFromPane(m_pActivePane);
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
  Rect original_bounds(rect);
  ::AdjustWindowRectEx(original_bounds.ptr(), dwStyle, TRUE, dwExStyle);

  auto const window_bounds = gfx::Rect(
      original_bounds.origin(),
      gfx::Size(original_bounds.width() + ::GetSystemMetrics(SM_CXVSCROLL) + 10,
                original_bounds.height()));

  Rect workarea_bounds;
  ::SystemParametersInfo(SPI_GETWORKAREA, 0, workarea_bounds.ptr(), 0);

  native_window()->CreateWindowEx(
      dwExStyle, dwStyle, L"", nullptr,
      gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      gfx::Size(window_bounds.width(), workarea_bounds.height() * 4 / 5));
}

void Frame::DidCreateNativeWindow() {
  views::FrameList::instance()->AddFrame(this);
  ::DragAcceptFiles(*native_window(), TRUE);

  title_bar_->Realize(*native_window());

  // TODO(yosi) How do we determine height of TabStrip?
  auto const tab_strip_height = tab_strip_->GetPreferreSize().height();
  const auto close_button_height = ::GetSystemMetrics(SM_CYSIZE);
  tab_strip_->SetBounds(Rect(0, close_button_height,
                             bounds().width(),
                             close_button_height + 4 + tab_strip_height));

  SetLayer(new ui::RootLayer(this));

  auto const pane_bounds = GetPaneRect();
  for (auto& pane: m_oPanes) {
    pane.SetBounds(pane_bounds);
  }

  message_view_->SetBounds(gfx::Rect(pane_bounds.bottom_left(),
                                     bounds().bottom_right()));

  {
    auto const size = metrics_view_->bounds().size();
    metrics_view_->SetBounds(gfx::Rect(
        pane_bounds.bottom_right() - size, size));
  }

  // Create message view, panes and tab strip.
  views::Window::DidCreateNativeWindow();
  layer()->SetTopMostLayer(metrics_view_->layer());

  for (auto& pane: m_oPanes) {
    AddTab(&pane);
  }

  if (m_oPanes.GetFirst())
    m_oPanes.GetFirst()->Activate();

  ::SetWindowPos(AssociatedHwnd(), nullptr, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER |
                 SWP_NOREDRAW | SWP_NOSIZE | SWP_FRAMECHANGED);

  //ui::Compositor::instance()->CommitIfNeeded();
}

void Frame::DidRemoveChildWidget(const ui::Widget& widget) {
  views::Window::DidRemoveChildWidget(widget);
  auto const pane = const_cast<Pane*>(widget.as<Pane>());
  if (!pane)
    return;
  m_oPanes.Delete(pane);
  if (first_child())
    return;
  DCHECK(m_oPanes.IsEmpty());
  DestroyWidget();
}

void Frame::DidChangeBounds() {
  views::Window::DidChangeBounds();
  const auto pane_bounds = GetPaneRect();

  tab_strip_->SetBounds(gfx::Rect(
      tab_strip_->bounds().origin(),
      gfx::Size(bounds().width(), tab_strip_->bounds().height())));

  if (tab_strip_layer_)
    tab_strip_layer_->SetBounds(gfx::RectF(tab_strip_->bounds()));

  // Display resizing information.
  if (message_view_) {
    auto const message_view_bounds = gfx::Rect(pane_bounds.bottom_left(),
                                               bounds().bottom_right());
    message_view_->SetBounds(message_view_bounds);
    message_view_->SetMessage(base::StringPrintf(L"Resizing... %dx%d",
        bounds().width(), bounds().height()));
  }

  for (auto& pane : m_oPanes) {
    pane.SetBounds(pane_bounds);
  }

  metrics_view_->SetBounds(gfx::Rect(
      pane_bounds.bottom_right() - metrics_view_->bounds().size(),
      metrics_view_->bounds().size()));

  ui::Compositor::instance()->CommitIfNeeded();

  DrawForResize();
}

void Frame::DidSetFocus(ui::Widget* widget) {
  views::Window::DidSetFocus(widget);
  if (!m_pActivePane) {
    m_pActivePane = m_oPanes.GetFirst();
    if (!m_pActivePane)
      return;
  }
  m_pActivePane->RequestFocus();
  FOR_EACH_OBSERVER(views::FrameObserver, observers_, DidActiveFrame(this));
}

LRESULT Frame::OnMessage(uint const uMsg, WPARAM const wParam,
                         LPARAM const lParam) {
  switch (uMsg) {
    case WM_DWMCOMPOSITIONCHANGED:
      CompositionState::Update(*native_window());
      // FALLTHROUGH

    case WM_ACTIVATE: {
        MARGINS margins;
        margins.cxLeftWidth = 0;
        margins.cxRightWidth = 0;
        margins.cyBottomHeight = 0;
        margins.cyTopHeight = -1;
        auto const hr = ::DwmExtendFrameIntoClientArea(*native_window(),
                                                       &margins);
        if (FAILED(hr)) {
          DEBUG_PRINTF("DwmExtendFrameIntoClientArea hr=0x%08X\n", hr);
        }
      }
      break;

    case WM_CLOSE:
      Application::instance()->view_event_handler()->QueryClose(window_id());
      return 0;

    case WM_DROPFILES:
      onDropFiles(reinterpret_cast<HDROP>(wParam));
      DEBUG_PRINTF("WM_DROPFILES\n");
      break;

    case WM_EXITSIZEMOVE:
      message_view_->SetMessage(base::string16());
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
      if (::DwmDefWindowProc(*native_window(), uMsg, wParam, lParam, &lResult))
        return lResult;
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

  return Widget::OnMessage(uMsg, wParam, lParam);
}

void Frame::OnMouseMoved(const ui::MouseEvent& event)  {
  views::Window::OnMouseMoved(event);
  drag_controller_->OnMouseMoved(event);
}

void Frame::OnMousePressed(const ui::MouseEvent& event)  {
  views::Window::OnMousePressed(event);
  drag_controller_->OnMousePressed(event);
}

void Frame::OnMouseReleased(const ui::MouseEvent& event)  {
  views::Window::OnMouseReleased(event);
  drag_controller_->OnMouseReleased(event);
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
  Widget::WillDestroyWidget();
  views::FrameList::instance()->RemoveFrame(this);
}

void Frame::WillRemoveChildWidget(const Widget& widget) {
  views::Window::WillRemoveChildWidget(widget);
  if (!is_realized())
    return;
  auto const pane = const_cast<Pane*>(widget.as<Pane>());
  if (!pane)
    return;
  auto const tab_index = getTabFromPane(pane);
  DCHECK_GE(tab_index, 0);
  tab_strip_->DeleteTab(tab_index);
}

// views::TabStripDelegate
void Frame::DidClickTabCloseButton(int tab_index) {
  if (m_oPanes.GetFirst() == m_oPanes.GetLast()) {
    // If this window has only one pane, we destroy this window.
    Application::instance()->view_event_handler()->QueryClose(
        window_id());
    return;
  }

  if (auto const pane = getPaneFromTab(tab_index))
    pane->DestroyWidget();

  LOG(ERROR) << "There is no tab[" << tab_index << "]";
}

void Frame::DidThrowTab(LPARAM lParam) {
  auto const pane = reinterpret_cast<Pane*>(lParam);
  auto const edit_pane = pane->as<EditPane>();
  if (!edit_pane)
    return;
  Application::instance()->view_event_handler()->DidDropWidget(
      edit_pane->GetActiveWindow()->window_id(),
      views::kInvalidWindowId);
}

base::string16 Frame::GetTooltipTextForTab(int tab_index) {
  auto const pane = getPaneFromTab(static_cast<int>(tab_index));
  if (!pane)
    return base::string16();

  auto const window = pane->GetWindow();
  if (!window)
    return base::string16();

  auto const tab_data = TabDataSet::instance()->GetTabData(
      window->window_id());
  return tab_data ? tab_data->tooltip : base::string16();
}

void Frame::OnDropTab(LPARAM lParam) {
  auto const pane = reinterpret_cast<Pane*>(lParam);
  if (pane->parent_node() == this)
    return;
  auto const edit_pane = pane->as<EditPane>();
  if (!edit_pane)
    return;
  Application::instance()->view_event_handler()->DidDropWidget(
      edit_pane->GetActiveWindow()->window_id(),
      window_id());
}

// views::Window
bool Frame::OnIdle(int const hint) {
  metrics_view_->RecordTime();
  views::MetricsView::TimingScope timing_scope(metrics_view_);

  DEFINE_STATIC_LOCAL(base::Time, busy_start_at, ());
  static bool busy;
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked()) {
    auto const now = base::Time::Now();
    if (!busy) {
      busy = true;
      busy_start_at = now;
    } else if (auto const delta = (now - busy_start_at).InSeconds()) {
      message_view_->SetMessage(base::StringPrintf(
        L"Script runs %ds. Ctrl+Break to terminate script.",
        delta));
    }
    return true;
  }

  metrics_view_->UpdateView();
  message_view_->UpdateView();

  busy = false;
  if (!pending_update_rect_.empty()) {
    gfx::Rect rect;
    std::swap(pending_update_rect_, rect);
    SchedulePaintInRect(rect);
  }
  auto const active_pane = GetActivePane();
  if (!active_pane)
    return false;
  if (!hint)
    updateTitleBar();
  auto more = false;
  for (auto child : active_pane->child_nodes()) {
    if (auto const window = child->as<Window>())
      more |= window->OnIdle(hint);
  }
  return more;
}
