// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/content/buffer_list_window.h"

#include "base/win/naitive_window.h"
#include "evita/cm_CmdProc.h"
#include "evita/ed_Mode.h"

#include "evita/vi_defs.h"
#include "evita/vi_Application.h"
#include "evita/vi_Buffer.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"

#define DEBUG_FOCUS _DEBUG

namespace Command {
uint TranslateKey(uint);
DEFCOMMAND(KillBuffers);
} // namespace Command

namespace {

class EnumItem {
  private: HWND   m_hwnd;
  private: int    m_iItem;
  private: uint   m_rgf;

  public: EnumItem(HWND hwnd, uint rgf = LVNI_SELECTED)
      : m_hwnd(hwnd),
        m_iItem(-1),
        m_rgf(rgf) {
    next();
  }

  public: bool AtEnd() const { return m_iItem < 0; }
  public: int Get() const { ASSERT(!AtEnd()); return m_iItem; }
  public: void Next() { ASSERT(!AtEnd()); next(); }

  private: void next() {
    m_iItem = ListView_GetNextItem(m_hwnd, m_iItem, m_rgf);
  }
};

static HCURSOR loadCursor(HCURSOR* inout_hCursor,
                          const char16* pwszDll,
                          uint nId) {
  if (auto hCursor = *inout_hCursor)
    return hCursor;

  auto const hDll = ::LoadLibraryEx(pwszDll, nullptr,
                                    LOAD_LIBRARY_AS_DATAFILE);
  if (!hDll)
    return nullptr;
  auto const hCursor = ::LoadCursor(hDll, MAKEINTRESOURCE(nId));
  ::FreeLibrary(hDll);
  *inout_hCursor = hCursor;
  return hCursor;
} // loadCursor

}  // naemspace

namespace content {

HCURSOR BufferListWindow::sm_hDragCursor;
Command::KeyBinds* BufferListWindow::sm_pKeyBinds;

//////////////////////////////////////////////////////////////////////
//
// BufferListWindow
//

BufferListWindow::BufferListWindow()
    : ALLOW_THIS_IN_INITIALIZER_LIST(
          CommandWindow_(widgets::NaitiveWindow::Create(*this))),
      m_pDragItem(nullptr),
      m_hwndListView(nullptr) {
}

// Activates the first selected buffer in-place.
void BufferListWindow::ActivateBuffers(bool is_new_frame) {
  LVITEM oItem;
  oItem.iSubItem = 0;
  oItem.mask = LVIF_PARAM;
  foreach (EnumItem, oEnum, m_hwndListView) {
    oItem.iItem = oEnum.Get();
    unless (ListView_GetItem(m_hwndListView, &oItem)) continue;
    auto const buffer = reinterpret_cast<Buffer*>(oItem.lParam);

    // Make sure buffer is alive.
    for (auto& runner: Application::Get()->buffers()) {
      if (runner == buffer) {
        if (is_new_frame) {
          auto const frame = Application::Get()->CreateFrame();
          frame->AddWindow(new TextEditWindow(buffer));
          frame->Realize();
        } else {
          frame().ShowBuffer(buffer);
        }
        return;
      }
    }
  }
}

int CALLBACK BufferListWindow::compareItems(LPARAM a, LPARAM b, LPARAM) {
  auto const pa = reinterpret_cast<Buffer*>(a);
  auto const pb = reinterpret_cast<Buffer*>(b);
  return ::lstrcmpW(pa->GetName(), pb->GetName());
}

// Creates host window for ListViewControl
void BufferListWindow::CreateNaitiveWindow() const {
  naitive_window()->CreateWindowEx(0, WS_CHILD | WS_VISIBLE,
                                   L"Buffer List",
                                   container_widget().AssociatedHwnd(),
                                   rect().left_top(), rect().size());
}

void BufferListWindow::dragFinish(POINT pt) {
  if (!m_pDragItem)
    return;
  auto const buffer = m_pDragItem;
  dragStop();
  if (auto const pane = Application::Get()->FindPane(AssociatedHwnd(), pt)) {
    // Drop to pane contains specified point.
    pane->GetFrame()->ShowBuffer(buffer);
  } else {
    // Create new fame
    auto const frame = Application::Get()->CreateFrame();
    frame->AddWindow(new TextEditWindow(buffer));
    frame->Realize();
  }
}

void BufferListWindow::dragMove(POINT pt) {
  if (!m_pDragItem)
    return;

  if (::GetCapture() != AssociatedHwnd()) {
    dragStop();
    return;
  }

  auto const pPane = Application::Get()->FindPane(AssociatedHwnd(), pt);

  HCURSOR hCursor;
  if (pPane && pPane->Is<EditPane>())
      hCursor = loadCursor(&sm_hDragCursor, L"ole32.dll", 3);
  else
      hCursor = ::LoadCursor(nullptr, IDC_NO);

  ::SetCursor(hCursor);
}

void BufferListWindow::dragStart(int iItem) {
  if (m_pDragItem)
    return;

  LVITEM oItem;
  oItem.iItem    = iItem;
  oItem.iSubItem = 0;
  oItem.mask     = LVIF_PARAM;
  if (!ListView_GetItem(m_hwndListView, &oItem))
    return;

  auto const pBuffer = reinterpret_cast<Buffer*>(oItem.lParam);
  for (auto& runner: Application::Get()->buffers()) {
    if (runner == pBuffer) {
        m_pDragItem = pBuffer;
        ::SetCursor(loadCursor(&sm_hDragCursor, L"ole32.dll", 3));
        ::SetCapture(AssociatedHwnd());
        return;
    }
  }
}

void BufferListWindow::dragStop() {
  ::ReleaseCapture();
  m_pDragItem = nullptr;
}

base::string16 BufferListWindow::GetTitle(size_t max_length) const {
  base::string16 title(L"*Buffer List*");
  return std::move(
      title.size() <= max_length ? title : title.substr(0, max_length));
}

void BufferListWindow::MakeSelectionVisible() {
  // TODO(yosi) Implement BufferListWindow::MakeSelectionVisible()
}

Command::KeyBindEntry* BufferListWindow::MapKey(uint nKey) {
  if (!sm_pKeyBinds) {
    sm_pKeyBinds = new Command::KeyBinds;
    sm_pKeyBinds->Bind(VK_DELETE | 0x100, Command::KillBuffers);
  }

  Command::KeyBindEntry* pEntry = sm_pKeyBinds->MapKey(nKey);
  if (pEntry)
    return pEntry;
  return Command::g_pGlobalBinds->MapKey(nKey);
}

void BufferListWindow::DidChangeHierarchy() {
  ::SetParent(m_hwndListView, AssociatedHwnd());
}

void BufferListWindow::DidCreateNaitiveWindow() {
  DEBUG_WIDGET_PRINTF("shown=%d " DEBUG_RECT_FORMAT "\n", is_shown(),
      DEBUG_RECT_ARG(rect()));

  auto const dwExStyle = LVS_EX_DOUBLEBUFFER |
                         LVS_EX_FULLROWSELECT |
                         // LVS_EX_GRIDLINES |
                        LVS_EX_HEADERDRAGDROP |
                        LVS_EX_LABELTIP |
                        LVS_EX_UNDERLINEHOT;
                        // |= LVS_EX_TRACKSELECT;

  auto const dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT |
                       LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;

  m_hwndListView = ::CreateWindowEx(0, //dwExStyle
                                    WC_LISTVIEW,
                                    nullptr,           // title
                                    dwStyle,
                                    0, 0,
                                    rect().width(), rect().height(),
                                    *naitive_window(),
                                    reinterpret_cast<HMENU>(ListViewId),
                                    g_hInstance,
                                    nullptr);

  ListView_SetExtendedListViewStyleEx(m_hwndListView, dwExStyle, dwExStyle);
  ListView_SetImageList(m_hwndListView, Application::Get()->GetIconList(),
                        LVSIL_SMALL);

  struct ColumnDef {
    const char16* m_pwsz;
    int m_cx;
    int m_fmt;
  };

  static ColumnDef k_rgoColumn[] = {
    { L"Name",      150, LVCFMT_LEFT },
    { L"Size",       60, LVCFMT_RIGHT },
    { L"State",      60, LVCFMT_LEFT },
    { L"Saved At",  100, LVCFMT_LEFT },
    { L"File",      300, LVCFMT_LEFT },
  };

  LVCOLUMN oColumn;
  oColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  //oColumn.iSubItem = 0;
  for (const ColumnDef* p = k_rgoColumn;
       p < k_rgoColumn + lengthof(k_rgoColumn);
       p++) {
    oColumn.cx  = p->m_cx;
    oColumn.fmt = p->m_fmt;
    oColumn.pszText = const_cast<char16*>(p->m_pwsz);
    ListView_InsertColumn(m_hwndListView, p - k_rgoColumn, &oColumn);
    oColumn.iSubItem += 1;
  }

  Redraw();
}

void BufferListWindow::DidResize() {
  ASSERT(m_hwndListView);
  ::SetWindowPos(m_hwndListView, nullptr, 0, 0,
                 rect().width(), rect().height(),
                 SWP_NOZORDER | SWP_SHOWWINDOW);
}

void BufferListWindow::DidSetFocus() {
  ::SetFocus(m_hwndListView);
}

void BufferListWindow::onKeyDown(uint nVKey) {
  auto const nKey = Command::TranslateKey(nVKey);
  if (!nKey)
    return;
  Application::Get()->Execute(this, nKey, 0);
}

LRESULT BufferListWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_LBUTTONUP: {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      dragFinish(pt);
      return 0;
    }

    case WM_MOUSEMOVE: {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      dragMove(pt);
      return 0;
    }

    case WM_NOTIFY: {
      auto const hdr = reinterpret_cast<NMHDR*>(lParam);
      switch (hdr->code) {
        case LVN_BEGINDRAG:
          dragStart(reinterpret_cast<NMLISTVIEW*>(hdr)->iItem);
          break;

        case LVN_ITEMACTIVATE: {
          auto const active = reinterpret_cast<NMITEMACTIVATE*>(hdr);
          ActivateBuffers((active->uKeyFlags & LVKF_CONTROL) != 0);
          break;
        }

        case LVN_KEYDOWN:
          onKeyDown(reinterpret_cast<NMLVKEYDOWN*>(lParam)->wVKey);
          break;
      } // switch code
      return 0;
    } // WM_NOTIFY
  }
  return ParentClass::OnMessage(uMsg, wParam, lParam);
}

void BufferListWindow::OnPaint(const base::win::Rect) {
  ::InvalidateRect(m_hwndListView, nullptr, true);
  ::UpdateWindow(m_hwndListView);
}

void BufferListWindow::Redraw() {
  ListView_DeleteAllItems(m_hwndListView);

  for (auto& buffer: Application::Get()->buffers()) {
    LVITEM oItem;
    oItem.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;

    oItem.iItem = 0;

    oItem.iImage   = buffer.GetMode()->GetIcon();
    oItem.iSubItem = 0;
    oItem.lParam   = reinterpret_cast<LPARAM>(&buffer);
    oItem.pszText  = const_cast<char16*>(buffer.GetName());
    ListView_InsertItem(m_hwndListView, &oItem);

    oItem.mask = LVIF_TEXT;
    char16 wsz[100];

    // Size
    oItem.iSubItem = 1;
    ::wsprintf(wsz, L"%d", buffer.GetEnd());
    oItem.pszText = wsz;
    ListView_SetItem(m_hwndListView, &oItem);

    // State
    {
      auto it = buffer.windows().begin();
      if (it != buffer.windows().end())
        ++it;

      char16* pwsz = wsz;
      *pwsz++ = static_cast<char16>(buffer.IsModified() ? '*' : '-');
      *pwsz++ = static_cast<char16>(buffer.IsReadOnly() ? '%' : '-');
      *pwsz++ = static_cast<char16>(buffer.IsNotReady() ? '!' : '-');
      *pwsz++ = static_cast<char16>(
        it == buffer.windows().begin() ? '-' :
            it == buffer.windows().end() ? '1' : 'w');
      *pwsz = 0;

      oItem.iSubItem = 2;
      oItem.pszText = wsz;
      ListView_SetItem(m_hwndListView, &oItem);
    }

    // Last Saved
    {
      if (!*buffer.GetFileName()) {
         wsz[0] = 0;
      } else {
        // FIXME 2007-08-05 We should use localized date time format.
        FILETIME ft;
        ::FileTimeToLocalFileTime(buffer.GetLastWriteTime(), &ft);
        SYSTEMTIME st;
        ::FileTimeToSystemTime(&ft, &st);
        ::wsprintf(wsz, L"%d/%d/%d %02d:%02d:%02d", st.wMonth, st.wDay,
                   st.wYear, st.wHour, st.wMinute, st.wSecond);
      }
      oItem.iSubItem = 3;
      oItem.pszText = wsz;
      ListView_SetItem(m_hwndListView, &oItem);
  }

    // File
    {
      oItem.iSubItem = 4;
      oItem.pszText = const_cast<char16*>(buffer.GetFileName());
      ListView_SetItem(m_hwndListView, &oItem);
    }
  } // for each bufer

  ListView_SortItems(m_hwndListView, compareItems, nullptr);

  // TODO We should select the last active window rather than the first one.
  uint const state = LVIS_SELECTED | LVIS_FOCUSED;
  ListView_SetItemState(m_hwndListView, 0, state, state);
}

void BufferListWindow::UpdateStatusBar() const {
  frame().SetStatusBar(0, L"");
}

}  // namespace content

namespace Command {

using content::BufferListWindow;

DEFCOMMAND(KillBuffers) {
  auto const pBufferList = pCtx->GetWindow()->DynamicCast<BufferListWindow>();
  if (!pBufferList)
    return;

  auto fKilled = false;
  auto hwndList = pBufferList->GetListWindow();

  LVITEM oItem;
  oItem.iSubItem = 0;
  oItem.mask = LVIF_PARAM;
  foreach (EnumItem, oEnum, hwndList) {
    oItem.iItem = oEnum.Get();
    if (!ListView_GetItem(hwndList, &oItem))
      continue;
    auto const pBuffer = reinterpret_cast<Buffer*>(oItem.lParam);
    if (Application::Get()->KillBuffer(pBuffer))
      fKilled = true;
  }

  if (fKilled)
    pBufferList->Redraw();
}

DEFCOMMAND(ListBuffer) {
  for (auto& frame: Application::Get()->frames()) {
    for (auto& pane: frame.panes()) {
      if (auto const pPane = pane.as<BufferListWindow>()) {
        if (pPane->frame() != pCtx->GetFrame())
          pCtx->GetFrame()->AddWindow(pPane);
        pPane->Activate();
        pPane->Redraw();
        pPane->SetFocus();
        return;
      }
    }
  }

  auto const pPane = new BufferListWindow();
  pCtx->GetFrame()->AddWindow(pPane);
}

} // Command
