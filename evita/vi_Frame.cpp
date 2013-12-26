
#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Frame Window
// listener/winapp/vi_Frame.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Frame.cpp#5 $
//
#define DEBUG_DROPFILES 0
#define DEBUG_FOCUS     1
#define DEBUG_PAINT     0
#define DEBUG_REDRAW    0
#define DEBUG_WINDOWPOS 0
#include "./vi_Frame.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/tree/ancestors_or_self.h"
#include "common/win/native_window.h"
#include "./ctrl_TabBand.h"
#include "./ed_Mode.h"
#include "./gfx_base.h"
#include "./vi_defs.h"
#include "evita/editor/application.h"
#include "./vi_Buffer.h"
#include "./vi_EditPane.h"
#include "./vi_Pane.h"
#include "./vi_Selection.h"
#include "./vi_Style.h"
#include "./vi_TextEditWindow.h"
#include "./vi_util.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

extern uint g_nDropTargetMsg;

static int const kPaddingBottom = 0;
static int const kPaddingLeft = 0;
static int const kPaddingRight = 0;
static int const kPaddingTop = 0;
static int const k_edge_size = 0;

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

Pane* AsPane(const widgets::Widget& widget) {
  // warning C4946: reinterpret_cast used between related classes: 
  // 'class1' and 'class2'
  #pragma warning(suppress: 4946)
  return reinterpret_cast<Pane*>(&const_cast<widgets::Widget&>(widget));
}

} // namespace

#define USE_TABBAND_EDGE 0
extern uint g_TabBand__TabDragMsg;

Frame::Frame()
    : ALLOW_THIS_IN_INITIALIZER_LIST(
          widgets::ContainerWidget(widgets::NativeWindow::Create(*this))),
      gfx_(new gfx::Graphics()),
      m_hwndTabBand(nullptr),
      m_pActivePane(nullptr) {
  ::ZeroMemory(m_rgpwszMessage, sizeof(m_rgpwszMessage));
}

Frame::~Frame() {
  for (auto i = 0; i < MessageLevel_Limit; i++) {
    delete[] m_rgpwszMessage[i];
  }
}

Frame::operator HWND() const {
  return *native_window();
}

v8_glue::ScriptWrapperInfo* Frame::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::ScriptWrapperInfo, wrapper_info, ("Frame"));
  return &wrapper_info;
}

bool Frame::Activate() {
  return ::SetForegroundWindow(*native_window());
}

void Frame::AddPane(Pane* const pane) {
  ASSERT(!!pane);
  ASSERT(!pane->GetFrame());
  ASSERT(!pane->is_realized());
  m_oPanes.Append(this, pane);
  AppendChild(*pane);
  if (is_realized()) {
    pane->Realize(GetPaneRect());
    AddTab(pane);
  }
}

// Adopt pane in another frame into this frame.
void Frame::AdoptPane(Pane* const pane) {
  ASSERT(!!pane);
  ASSERT(pane->is_realized());

  auto const frame = pane->GetFrame();
  ASSERT(!!frame);
  ASSERT(frame != this);
  ASSERT(frame->is_realized());
  pane->SetParentWidget(*this);
}

void Frame::AddTab(Pane* const pane) {
  ASSERT(is_realized());
  ASSERT(pane->is_realized());
  ASSERT(m_hwndTabBand);
  TCITEM tab_item;
  tab_item.mask = TCIF_TEXT | TCIF_PARAM;
  char16 name[100];
  pane->GetTitle(name, arraysize(name));
  tab_item.pszText = name;
  tab_item.lParam = reinterpret_cast<LPARAM>(pane);

  if (auto const edit_pane = pane->DynamicCast<EditPane>()) {
    if (auto const buffer = edit_pane->GetBuffer()) {
      tab_item.iImage = buffer->GetMode()->GetIcon();
      if (tab_item.iImage != -1)
        tab_item.mask |= TCIF_IMAGE;
    }
  }

  auto const new_tab_item_index = TabCtrl_GetItemCount(m_hwndTabBand);
  TabCtrl_InsertItem(m_hwndTabBand, new_tab_item_index, &tab_item);
  TabCtrl_SetCurSel(m_hwndTabBand, new_tab_item_index);
}

void Frame::AddWindow(content::ContentWindow* window) {
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

  std::unique_ptr<EditPane> new_pane(new EditPane(window));
  AddPane(new_pane.release());
}

bool Frame::canClose() {
  if (Application::Get()->HasMultipleFrames()) {
    // Destroy this frame since we have multiple frames.
    return true;
  }

  // Can we exist applicaiton safety?
  return Application::Get()->CanExit();
}

void Frame::DidActivatePane(Pane* const pane) {
  auto const tab_index = getTabFromPane(pane);
  if (tab_index < 0)
    return;
  auto const selected_index = TabCtrl_GetCurSel(m_hwndTabBand);
  #if DEBUG_FOCUS
   DEBUG_WIDGET_PRINTF("selected_index=%d"
                       " cur=" DEBUG_WIDGET_FORMAT ".focus=%d"
                       " new=" DEBUG_WIDGET_FORMAT ".focus=%d at %d\n",
        selected_index,
        DEBUG_WIDGET_ARG(m_pActivePane), m_pActivePane->has_focus(),
        DEBUG_WIDGET_ARG(pane), pane->has_focus(), tab_index);
  #endif

  if (tab_index != selected_index)
    TabCtrl_SetCurSel(m_hwndTabBand, tab_index);
}

void Frame::DidAddChildWidget(const widgets::Widget& widget) {
  auto const pane = AsPane(widget);
  m_oPanes.Append(this, pane);
  if (!is_realized())
    return;
  if (pane->is_realized())
    pane->ResizeTo(GetPaneRect());
  else
    pane->Realize(GetPaneRect());
  AddTab(pane);
}

void Frame::DidChangeTabSelection(int selected_index) {
  auto const pane = getPaneFromTab(selected_index);
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("Start selected_index=%d"
        " cur=" DEBUG_WIDGET_FORMAT
        " new=" DEBUG_WIDGET_FORMAT "\n",
        selected_index,
        DEBUG_WIDGET_ARG(m_pActivePane),
        DEBUG_WIDGET_ARG(pane));
  #endif
  if (!pane) {
    #if DEBUG_FOCUS
      DEBUG_WIDGET_PRINTF("selected_index(%d) doesn't have pane!\n",
          selected_index);
    #endif
    return;
  }
  if (m_pActivePane == pane) {
    #if DEBUG_FOCUS
      DEBUG_WIDGET_PRINTF("Active pane isn't changed. why?\n",
          selected_index);
    #endif
    return;
  }
  if (m_pActivePane) {
    m_pActivePane->Hide();
  } else {
    #if DEBUG_FOCUS
      DEBUG_WIDGET_PRINTF("Why we don't have acitve pane?\n");
    #endif
  }
  m_pActivePane = pane;
  pane->Show();
  pane->Activate();
  updateTitleBar();
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("End selected_index=%d"
        " cur=" DEBUG_WIDGET_FORMAT
        " new=" DEBUG_WIDGET_FORMAT "\n",
        selected_index,
        DEBUG_WIDGET_ARG(m_pActivePane),
        DEBUG_WIDGET_ARG(pane));
  #endif
}

void Frame::DidCreateNativeWindow() {
  ::DragAcceptFiles(*native_window(), TRUE);

  {
    m_hwndTabBand = ::CreateWindowEx(
        0,
        L"TabBandClass",
        nullptr,
        WS_CHILD | WS_VISIBLE | TCS_TOOLTIPS,
        0, 0, 0, 0,
        *native_window(),
        reinterpret_cast<HMENU>(CtrlId_TabBand),
        g_hInstance,
        nullptr);

    ::SendMessage(
       m_hwndTabBand,
       TCM_SETIMAGELIST,
       0,
       reinterpret_cast<LPARAM>(
          Application::Get()->GetIconList()));

    RECT rc;
    ::GetWindowRect(m_hwndTabBand, &rc);
    m_cyTabBand = rc.bottom - rc.top;
  }

  m_oStatusBar.Realize(*native_window(), CtrlId_StatusBar);
  m_oTitleBar.Realize(*native_window());

  CompositionState::Update(*native_window());
  gfx_->Init(*native_window());

  ASSERT(!m_oPanes.IsEmpty());
  auto const pane_rect = GetPaneRect();
  for (auto& pane: m_oPanes) {
    pane.Realize(pane_rect);
    AddTab(&pane);
  }

  if (m_oPanes.GetFirst())
    m_oPanes.GetFirst()->Activate();
}

void Frame::DidDestroyWidget() {
  if (!has_script_reference())
    delete this;
}

void Frame::DidRemoveChildWidget(const widgets::Widget& widget) {
  auto const pane = AsPane(widget);
  m_oPanes.Delete(pane);
  if (m_oPanes.IsEmpty())
    Destroy();
}

void Frame::DidResize() {
  // Tab Band
  ::SetWindowPos(m_hwndTabBand, nullptr, rect().left, rect().top,
                 rect().width(), m_cyTabBand, SWP_NOZORDER);

  // Status Bar
  //  message, code page, newline, line num, column, char, ins/ovr
  if (m_oStatusBar) {
    ::SetWindowPos(m_oStatusBar, nullptr, rect().left,
                   rect().bottom - m_oStatusBar.GetCy(), rect().width(),
                   m_oStatusBar.GetCy(), SWP_NOZORDER);

    ::SendMessage(m_oStatusBar, SB_SIMPLE, 1, 0);

    char16 wsz[100];
    ::wsprintf(wsz, L"Resizing... %dx%d",
        rect().right - rect().left,
        rect().bottom - rect().top);

    ::SendMessage(m_oStatusBar, SB_SETTEXT, SB_SIMPLEID | SBT_NOBORDERS,
                  reinterpret_cast<LPARAM>(wsz));
  }

  gfx_->Resize(rect());
  {
    gfx::Graphics::DrawingScope drawing_scope(*gfx_);
    (*gfx_)->Clear(gfx::ColorF(gfx::ColorF::Green));

    const auto rc = GetPaneRect();
    for (auto& pane: m_oPanes) {
      pane.ResizeTo(rc);
    }
  }

  Paint();
}

void Frame::DidSetFocus() {
  if (!m_pActivePane) {
    m_pActivePane = m_oPanes.GetFirst();
    if (!m_pActivePane)
      return;
  }
  Application::Get()->SetActiveFrame(this);
  m_pActivePane->SetFocus();
}

Frame* Frame::FindFrame(const widgets::Widget& widget) {
  for (auto& ancestor: common::tree::ancestors_or_self(widget)) {
    if (ancestor.is<Frame>())
      return const_cast<Widget&>(ancestor).as<Frame>();
  }
  return nullptr;
}

Pane* Frame::GetActivePane() {
  if (m_pActivePane && m_pActivePane->GetActiveTick())
    return m_pActivePane;

  auto pActive = m_oPanes.GetFirst();
  for (auto& pane: m_oPanes) {
    if (pActive->GetActiveTick() < pane.GetActiveTick())
      pActive = &pane;
  }

  return pActive;
}

int Frame::GetCxStatusBar() const {
  auto const cx = rect().right - rect().left -
      ::GetSystemMetrics(SM_CXVSCROLL);  // remove size grip
  return cx;
}

static Pane* getPaneAt(HWND hwnd, int const index) {
  TCITEM tab_item;
  tab_item.mask = TCIF_PARAM;
  if (!TabCtrl_GetItem(hwnd, index, &tab_item))
      return nullptr;
  return reinterpret_cast<Pane*>(tab_item.lParam);
}

Pane* Frame::getPaneFromTab(int const index) const {
  auto const present = getPaneAt(m_hwndTabBand, index);
  if (!present)
    return nullptr;

  for (auto& pane: m_oPanes) {
    if (pane == present)
      return present;
  }

  return nullptr;
}

int Frame::getTabFromPane(Pane* const pane) const {
  auto index = 0;
  while (auto const present = getPaneAt(m_hwndTabBand, index)) {
    if (present == pane)
      return index;
    ++index;
  }
  return -1;
}

gin::ObjectTemplateBuilder Frame::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetEmptyObjectTemplateBuilder(isolate)
      .SetMethod("activate", &Frame::Activate);
}

Rect Frame::GetPaneRect() const {
  return Rect(rect().left + k_edge_size + kPaddingLeft,
              rect().top + m_cyTabBand + k_edge_size * 2 + kPaddingLeft,
              rect().right - k_edge_size + kPaddingRight,
              rect().bottom - m_oStatusBar.GetCy() + k_edge_size +
                  kPaddingBottom);
}

const char16* Frame::getToolTip(NMTTDISPINFO* const pDisp) const {
  auto const pPane = getPaneFromTab(static_cast<int>(pDisp->hdr.idFrom));
  if (!pPane)
    return L"";

  auto const pEdit = pPane->DynamicCast<EditPane>();
  if (!pEdit)
    return pPane->GetName();

  auto const pBuffer = pEdit->GetBuffer();
  if (!pBuffer)
    return pPane->GetName();

  const char16* pwszSave;
  char16 wszSave[100];
  if (!*pBuffer->GetFileName()) {
    pwszSave = L"Not saved";
  } else {
    // FIXME 2007-08-05 We should use localized date time format.
    FILETIME ft;
    ::FileTimeToLocalFileTime(pBuffer->GetLastWriteTime(), &ft);
    SYSTEMTIME st;
    ::FileTimeToSystemTime(&ft, &st);
    ::wsprintf(wszSave, L"%d/%d/%d %02d:%02d:%02d",
        st.wMonth,
        st.wDay,
        st.wYear,
        st.wHour,
        st.wMinute,
        st.wSecond);
    pwszSave = wszSave;
  }

  //char16 wszMod[100];
  auto const pwszModified =
    !pBuffer->IsModified()
        ? L"Not modified"
        : pBuffer->GetNoSave()
            ? L"Modified"
            : L"Not saved";

  ::wsprintf(m_wszToolTip,
      L"Name: %s\r\n"
      L"File: %s\r\n"
      L"Save: %s\r\n"
      L"%s\r\n",
      pBuffer->GetName(),
      !*pBuffer->GetFileName()
          ? L"No file"
          : pBuffer->GetFileName(),
      pwszSave,
      pwszModified);

    return m_wszToolTip;
}

void Frame::onDropFiles(HDROP const hDrop) {
  uint nIndex = 0;
  for (;;) {
    char16 wsz[MAX_PATH + 1];
    auto const cwch = ::DragQueryFile(hDrop, nIndex, wsz, lengthof(wsz));
    if (!cwch)
      break;

    auto const pBuffer = Application::Get()->Load(wsz);
    Pane* pPane = nullptr;
    for (auto& pane: m_oPanes) {
      auto const pEditPane = pane.DynamicCast<EditPane>();
      if (!pEditPane) {
        continue;
      }

      if (pEditPane->GetBuffer () == pBuffer) {
        pPane = pEditPane;
        break;
      }
    }

    if (!pPane) {
      pPane = new EditPane(pBuffer);
      AddPane(pPane);
    }

    pPane->Activate();

    nIndex += 1;
  }

  ::DragFinish(hDrop);
  Activate();
}

void Frame::Paint() {
#if USE_TABBAND_EDGE
  {
    RECT rc = rect_;

    #if DEBUG_REDRAW
      DEBUG_PRINTF("frame=%p %dx%d+%d+%d\n",
          this,
          rc.right - rc.left, rc.bottom - rc.top, rc.left, rc.top);
     #endif

     rc.top += m_cyTabBand;
     rc.bottom = rc.top + k_edge_size;

     {
       auto const color = static_cast<COLORREF>(
          ::SendMessage(m_hwndTabBand, WM_USER, 0, 0));

        auto const hBrush = ::CreateSolidBrush(color);

        ::FillRect(hdc, &rc, hBrush);
        ::DeleteObject(hBrush);
     }
  }

  {
    RECT rc;
    GetPaneRect(&rc);
    rc.top -= k_edge_size;
    rc.left -= k_edge_size;
    rc.right += k_edge_size;
    rc.bottom +=  k_edge_size;
    ::DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
  }
#endif //USE_TABBAND_EDGE
}

/// <summary>
///   Idle processing
/// </summary>
bool Frame::OnIdle(uint const nCount) {
  class Local {
    public: static void HandleObsoleteBuffer(Buffer* const pBuffer) {
      // Prevent further obsolete checking.
      pBuffer->SetObsolete(Edit::Buffer::Obsolete_Ignore);

      auto const iAnswer = Application::Get()->Ask(
          MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST,
          IDS_ASK_REFRESH,
          pBuffer->GetName());

      switch (iAnswer) {
        case IDNO:
          break;

        case IDYES:
          for (auto& window: pBuffer->windows())
            window.GetSelection()->PrepareForReload();
          pBuffer->Load(pBuffer->GetFileName());
          break;

        default:
         CAN_NOT_HAPPEN();
      }
    }
  }; // Local

  if (!nCount) {
    if (has_focus()) {
      updateTitleBar();

      if (auto const pEditPane = m_pActivePane->DynamicCast<EditPane>()) {
        auto const pBuffer = pEditPane->GetBuffer();

        switch (pBuffer->GetObsolete()) {
          case Buffer::Obsolete_Checking:
          case Buffer::Obsolete_Ignore:
          case Buffer::Obsolete_Unknown:
            break;

          case Buffer::Obsolete_No:
            pBuffer->UpdateFileStatus();
            break;

          case Buffer::Obsolete_Yes:
            Local::HandleObsoleteBuffer(pBuffer);
            break;

          default:
             CAN_NOT_HAPPEN();
        }
      }
    }
  }

  return ContainerWidget::OnIdle(nCount);
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
        margins.cyTopHeight = CompositionState::IsEnabled() ? m_cyTabBand : 0;
        auto const hr = ::DwmExtendFrameIntoClientArea(*native_window(),
                                                       &margins);
        if (FAILED(hr)) {
          DEBUG_PRINTF("DwmExtendFrameIntoClientArea hr=0x%08X\n", hr);
        }
      }
      break;

    case WM_CLOSE:
      if (canClose())
        break;
      return 0;

    case WM_DROPFILES:
      onDropFiles(reinterpret_cast<HDROP>(wParam));
      DEBUG_PRINTF("WM_DROPFILES\n");
      break;

    case WM_GETMINMAXINFO: {
      auto pMinMax = reinterpret_cast<MINMAXINFO*>(lParam);
      pMinMax->ptMinTrackSize.x = 200;
      pMinMax->ptMinTrackSize.y = 200;
      return 0;
    }

    case WM_NCHITTEST:
      if (CompositionState::IsEnabled()) {
        LRESULT lResult;
        if (::DwmDefWindowProc(*native_window(), uMsg, wParam, lParam, &lResult))
            return lResult;

        POINT const ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        RECT rcWindow;
        ::GetWindowRect(*native_window(), &rcWindow);

        RECT rcClient = { 0 };
        ::AdjustWindowRectEx(
            &rcClient,
            static_cast<DWORD>(
                ::GetWindowLong(*native_window(), GWL_STYLE)),
            false,
            static_cast<DWORD>(
                ::GetWindowLong(*native_window(), GWL_EXSTYLE)));

        if (ptMouse.y >= rcWindow.top
            && ptMouse.y < rcWindow.top - rcClient.top + m_cyTabBand) {
          return HTCAPTION;
        }
      }
      break;

    case WM_NOTIFY: {
      auto const pNotify = reinterpret_cast<NMHDR*>(lParam);
      if (TTN_NEEDTEXT == pNotify->code) {
          auto const p = reinterpret_cast<NMTTDISPINFO*>(lParam);
          ::SendMessage(p->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
          p->lpszText = const_cast<char16*>(getToolTip(p));
          return 0;
      }

      switch (pNotify->idFrom) {
        case CtrlId_TabBand:
          switch (pNotify->code) {
            case TABBAND_NOTIFY_CLOSE: {
              auto const tab_index = TabBandNotifyData::FromNmhdr(
                    pNotify)->tab_index_;
                if (auto const pPane = getPaneFromTab(tab_index))
                  pPane->Destroy();
                break;
            }

            case TABBAND_NOTIFY_QUERY_CLOSE:
              return HasMultiplePanes() || canClose();

            case TCN_SELCHANGE:
              DidChangeTabSelection(TabCtrl_GetCurSel(m_hwndTabBand));
              break;
          }
        }
        return 0;
    }

    case WM_VSCROLL: {
      auto const hwnd_scrollbar = reinterpret_cast<HWND>(lParam);
      auto const widget = reinterpret_cast<Widget*>(
        ::GetWindowLongPtr(hwnd_scrollbar, GWLP_USERDATA));
      if (widget) {
        widget->OnMessage(uMsg, wParam, lParam);
        return 0;
      }
      break;
    }

    default:
      if (uMsg == g_TabBand__TabDragMsg) {
        return onTabDrag(
            static_cast<TabBandDragAndDrop>(wParam),
            reinterpret_cast<HWND>(lParam));
      }

      // Handle a message from BufferListPane.
      if (g_nDropTargetMsg && g_nDropTargetMsg == uMsg)
        return reinterpret_cast<LRESULT>(GetActivePane());
      break;
  }

  return Widget::OnMessage(uMsg, wParam, lParam);
}

bool Frame::onTabDrag(TabBandDragAndDrop const eAction,
                      HWND const hwndTabBand) {
  auto const pFrom = Application::Get()->FindFrame(::GetParent(hwndTabBand));

  if (!pFrom) {
    // We should not be here.
    return false;
  }

  auto const pPane = pFrom->GetActivePane();
  if (!pPane) {
    // Why is pPane nullptr?
    return false;
  }

  switch (eAction) {
    case kDrop:
      if (this == pFrom)
        break;
      AdoptPane(pPane);
      break;

    case kHover:
      break;

    case kThrow: {
      auto const pNewFrame = Application::Get()->CreateFrame();
      pNewFrame->AdoptPane(pPane);
      pNewFrame->Realize();
      break;
    }

    default:
      CAN_NOT_HAPPEN();
  }

  return true;
}

/// <summary>
///   Realize this frame.
/// </summary>
void Frame::CreateNativeWindow() const {
  int const cColumns = 80;
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

  CompositionState::Update();
  if (CompositionState::IsEnabled())
    dwStyle |= WS_EX_COMPOSITED | WS_EX_LAYERED;

  auto& font = *FontSet::Get(&g_DefaultStyle)->FindFont('x');
  gfx::SizeF size(font.GetCharWidth('M') * cColumns,
                  font.height() * cRows);
  gfx::RectF rect(gfx::PointF(), gfx::FactorySet::AlignToPixel(size));
  Rect rc(rect);
  ::AdjustWindowRectEx(&rc, dwStyle, TRUE, dwExStyle);
  rc.right += ::GetSystemMetrics(SM_CXVSCROLL) + 10;

  Rect rcWork;
  ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

  native_window()->CreateWindowEx(
      dwExStyle, dwStyle, L"", nullptr,
      gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT),
      gfx::Size(rc.width(), rcWork.height() * 4 / 5));

  SetStatusBar(0, L"Ready");
}

void Frame::Realize() {
  RealizeTopLevelWidget();
}

/// <summary>
///   Remove all messages in status bar.
/// </summary>
void Frame::ResetMessages() {
  ::ZeroMemory(m_rgpwszMessage, sizeof(m_rgpwszMessage));
}

/// <summary>
///   Set status bar message on specified part.
/// </summary>
void Frame::SetStatusBar(int const ePart, const char16* const pwszMsg) const {
  ::SendMessage(m_oStatusBar, SB_SIMPLE, 0, 0);

  ::SendMessage(
      m_oStatusBar,
      SB_SETTEXT,
      static_cast<WPARAM>(ePart | SBT_NOBORDERS),
      reinterpret_cast<LPARAM>(pwszMsg));
}

/// <summary>
///   Set status bar formatted message on specified part.
/// </summary>
void Frame::SetStatusBarf(
    int const ePart,
    const char16* pwszFormat, ...) {
  char16 wsz[1024];

  va_list args;
  va_start(args, pwszFormat);
  ::wvsprintf(wsz, pwszFormat, args);
  va_end(args);
  SetStatusBar(ePart, wsz);
}

/// <summary>
///   Set parts width of status bar.
/// </summary>
void Frame::SetStatusBarParts(int const* const prgiPart, int const cParts) {
  m_oStatusBar.SetParts(prgiPart, cParts);
}

/// <summary>
///   Show or activate specified buffer on this frame.
/// </summary>
/// <param name="pBuffer">A buffer to display or activate.</param>
/// <returns>
///   True if buffer hasn't been displayed in this frame. False if associated
///   window is activated.
/// </returns>
bool Frame::ShowBuffer(Buffer* const pBuffer) {
  for (auto& pane: m_oPanes) {
    if (auto const edit_pane = pane.DynamicCast<EditPane>()) {
      if (edit_pane->GetBuffer() == pBuffer) {
        edit_pane->Activate();
        return false;
      }
    }
  }

  auto const pPane = new EditPane(pBuffer);
  AddPane(pPane);
  pPane->Activate();
  return true;
}

/// <summary>
///   Display specified message on status bar.
/// </summary>
void Frame::ShowMessage(
    MessageLevel const iLevel,
    uint const nFormatId, ...) {
  delete[] m_rgpwszMessage[iLevel];
  m_rgpwszMessage[iLevel] = nullptr;
  if (nFormatId) {
    char16 wszFormat[1024];
    ::LoadString(g_hResource, nFormatId, wszFormat, lengthof(wszFormat));

    char16 wsz[1024];

    va_list args;
    va_start(args, nFormatId);
    ::wvsprintf(wsz, wszFormat, args);
    va_end(args);

    auto const cwch = static_cast<size_t>(::lstrlenW(wsz));
    auto const pwsz = new char16[cwch + 1];
    myCopyMemory(pwsz, wsz, sizeof(char16) * (cwch + 1));
    m_rgpwszMessage[iLevel] = pwsz;
  }

  int i = MessageLevel_Limit;
  do {
    i -= 1;
    if (char16* pwsz = m_rgpwszMessage[i]) {
      SetStatusBar(0, pwsz);
      return;
    }
  } while (i > 0);
}

// [U]
/// <summary>
///   Updates title bar to display active buffer.
/// </summary>
void Frame::updateTitleBar() {
  char16 wsz[1024];
  m_pActivePane->GetTitle(wsz, lengthof(wsz));

  base::string16 title;
  title += base::string16(wsz);
  title += L" - ";
  title += Application::instance().title();
  m_oTitleBar.SetText(title.data(), title.length());

  m_pActivePane->UpdateStatusBar();
}

void Frame::WillDestroyWidget() {
  ContainerWidget::WillDestroyWidget();
  Application::instance().DeleteFrame(this);
}

void Frame::WillRemoveChildWidget(const Widget& widget) {
  if (!is_realized())
    return;
  auto const pane = AsPane(widget);
  auto const tab_index = getTabFromPane(pane);
  DCHECK_GE(tab_index, 0);
  TabCtrl_DeleteItem(m_hwndTabBand, tab_index);
}
