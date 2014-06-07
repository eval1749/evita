// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DRAG 0
#define DEBUG_HOVER 0
#define DEBUG_MESSAGE 0
#define DEBUG_TOOLTIP 0
#include "evita/views/tab_strip.h"

#include <dwmapi.h>

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/castable.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/events/event.h"
#include "evita/views/frame_list.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/vi_Frame.h"

namespace {

static HINSTANCE g_hInstance;

static void fillRect(const gfx::Canvas& gfx, int x, int y, int cx, int cy) {
  RECT rc;
  rc.left = x;
  rc.right = x + cx;
  rc.top = y;
  rc.bottom = y + cy;
  gfx::Brush brush(gfx, gfx::blackColor());
  gfx.FillRectangle(brush, rc);
}

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element : public common::Castable {
  DECLARE_CASTABLE_CLASS(Element, Castable);

  public: enum State {
    State_Normal,
    State_Selected,
  }; // State

  protected: State m_eState;
  protected: bool  m_fHover;
  protected: bool  m_fShow;
  protected: HIMAGELIST  m_hImageList;
  protected: Element* m_pParent;
  protected: RECT m_rc;

  // ctor
  protected: Element(Element* pParent);
  public: virtual ~Element() = default;

  protected: gfx::ColorF backgroundColor() const;

  // [D]
  public: virtual void Draw(const gfx::Canvas&) const = 0;

  // [G]
  public: bool GetHover() const { return m_fHover; }
  public: HIMAGELIST GetImageList() const;
  public: RECT*  GetRect() { return &m_rc; }
  public: Element* GetParent() const { return m_pParent; }
  public: State  GetState() const { return m_eState; }

  // [H]
  public: virtual Element* HitTest(POINT pt) const;

  // [I]
  public: void Invalidate(HWND hwnd);
  public: bool IsHover() const { return m_fHover; }
  public: bool IsSelected() const { return State_Selected == m_eState; }
  public: bool IsShow() const { return m_fShow; }

  // [S]
  public: bool SetHover(bool f);
  public: void SetImageList(HIMAGELIST hImageList);
  public: Element* SetParent(Element* p);
  public: State SetState(State e);
  public: bool Show(bool f);

  // [U]
  protected: virtual void update();
};

Element::Element(Element* pParent)
    : m_eState(State_Normal),
      m_fHover(false),
      m_fShow(true),
      m_hImageList(nullptr),
      m_pParent(pParent) {
  m_rc.left = m_rc.top = m_rc.right = m_rc.top = 0;
}

gfx::ColorF Element::backgroundColor() const {
  if (IsSelected())
      return gfx::whiteColor();
  if (IsHover())
      return gfx::sysColor(COLOR_3DHILIGHT, 0.8);
  return gfx::sysColor(COLOR_3DFACE, 0.5);
}

HIMAGELIST Element::GetImageList() const {
  const Element* pRunner = this;
  do {
    auto const hImageList = pRunner->m_hImageList;
    if (hImageList) {
      return hImageList;
    }

    pRunner = pRunner->GetParent();
  } while (pRunner);
  return nullptr;
}

Element* Element::HitTest(POINT pt) const {
  return IsShow() && ::PtInRect(&m_rc, pt) ? const_cast<Element*>(this) :
                                             nullptr;
}

void Element::Invalidate(HWND hwnd) {
  // TODO(yosi) We should use GFX version of invalidate rectangle.
  ::InvalidateRect(hwnd, &m_rc, false);
}

bool Element::SetHover(bool f) {
  return m_fHover = f;
}

void Element::SetImageList(HIMAGELIST hImageList) {
  m_hImageList = hImageList;
}

Element* Element::SetParent(Element* p) {
  return m_pParent = p;
}

Element::State Element::SetState(State e) {
  m_eState = e;
  update();
  return m_eState;
}

bool Element::Show(bool f) {
  return m_fShow = f;
}

void Element::update() {
}

//////////////////////////////////////////////////////////////////////
//
// TabStripImpl Design Parameters
//
enum TabStripImplDesignParams {
  k_cxMargin = 3,
  k_cyMargin = 2,
  k_cxListButton = 16,
  k_cxEdge = 2,
  k_cxBorder = 3,
  k_cxPad = 3,
  k_cxMinTab = 140,
  k_cyBorder = 5,
  k_cyIcon = 16,
};

//////////////////////////////////////////////////////////////////////
//
// CloseBox
//
class CloseBox final : public Element {
  DECLARE_CASTABLE_CLASS(CloseBox, Element);

  public: enum Design {
    Height = 16,
    Width = 17,
  };

  public: CloseBox(Element* pParent);
  public: virtual ~CloseBox() = default;

  // [D]
  private: void drawXMark(const gfx::Canvas& gfx, gfx::ColorF color) const;
  private: gfx::ColorF markColor() const;

  // Element
  public: virtual void Draw(const gfx::Canvas& gfx) const override;

  DISALLOW_COPY_AND_ASSIGN(CloseBox);
};

CloseBox::CloseBox(Element* pParent) : Element(pParent) {
}

void CloseBox::drawXMark(const gfx::Canvas& gfx, gfx::ColorF color) const {
  gfx::Brush brush(gfx, color);

  RECT rc = m_rc;
  rc.left += 4;
  rc.top  += 4;

  // 01234567890123
  // ----ooo---ooo--- 4
  // -----ooo-ooo---- 5
  // ------ooooo----- 6
  // -------ooo------ 7
  // -------ooo------ 8
  // ------ooooo----- 9
  // -----ooo-ooo---- 10
  // ----ooo---ooo--- 11
  #define hline(x, y, cx, cy) \
    gfx.FillRectangle( \
      brush, \
      m_rc.left + x, m_rc.top + y, \
      m_rc.left + x + cx, m_rc.top + y + cy);

  hline( 4, 4, 3, 1);
  hline(10, 4, 3, 1);

  hline( 5, 5, 3, 1);
  hline( 9, 5, 3, 1);

  hline( 6, 6, 5, 1);

  hline( 7, 7, 3, 2);  // center

  hline( 6, 9, 5, 1);

  hline( 5, 10, 3, 1);
  hline( 9, 10, 3, 1);

  hline( 4, 11, 3, 1);
  hline(10, 11, 3, 1);

  #undef hline
}

gfx::ColorF CloseBox::markColor() const {
  return IsHover() ? gfx::ColorF::DarkViolet : gfx::ColorF::DimGray;
}

void CloseBox::Draw(const gfx::Canvas& gfx) const {
  drawXMark(gfx, markColor());
}

//////////////////////////////////////////////////////////////////////
//
// Item
//  Represents tab item.
//
class Item final : public Element {
  DECLARE_CASTABLE_CLASS(Item, Element);

  private: enum Design {
    k_cxCloseBoxMargin = 3,
    k_cyCloseBoxMargin = 9,
    k_cyDescent = 4,
  };

  public: base::string16 label_;
  private: int tab_index_;
  public: int m_iImage;
  public: LPARAM m_lParam;
  private: RECT m_rcLabel;
  private: CloseBox m_closeBox;
  public: uint32_t m_rgfState;

  public: Item(Element* pParent, const TCITEM* pTcItem);
  public: virtual ~Item() = default;

  public: int tab_index() const { return tab_index_; }
  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  public: void ComputeLayout();
  private: void drawContent(const gfx::Canvas& gfx) const;
  private: void drawIcon(const gfx::Canvas& gfx) const;
  public: bool HasCloseBox() const;
  public: bool SetItem(const TCITEM* pTcItem);

  // Element
  public: virtual void Draw(const gfx::Canvas& gfx) const override;
  public: virtual Element* HitTest(POINT point) const override;
  private: void update() override;

  DISALLOW_COPY_AND_ASSIGN(Item);
};

Item::Item(Element* pParent, const TCITEM* pTcItem) :
    m_iImage(-1),
    tab_index_(0),
    m_rgfState(0),
    m_closeBox(this),
    Element(pParent) {
  SetItem(pTcItem);
}

void Item::ComputeLayout() {
  m_rcLabel = m_rc;

  auto const prc = m_closeBox.GetRect();
  *prc = m_rc;

  prc->right  -= k_cxCloseBoxMargin;
  prc->left = prc->right - CloseBox::Width;
  prc->top  += k_cyCloseBoxMargin;
  prc->bottom = prc->top + CloseBox::Height;

  m_rcLabel.right = prc->left;

  m_rcLabel.left += k_cxBorder + k_cxEdge;
  m_rcLabel.right -= k_cxBorder + k_cxEdge;
  m_rcLabel.top  += 6 + 4;
  m_rcLabel.bottom = m_rcLabel.top + 12;

  if (m_iImage >= 0) {
    m_rcLabel.left += 16 + 4;
  }
}

void Item::drawContent(const gfx::Canvas& gfx) const {
  drawIcon(gfx);

  // Label Text
  {
    RECT rc = m_rc;
    rc.left += 4 + 16 + 4;
    rc.right -= 4;
    rc.top += 8;
    rc.bottom = rc.bottom - 2;

    gfx::Brush brush(gfx, gfx::sysColor(COLOR_BTNTEXT));
    gfx.DrawText(*gfx.work<gfx::TextFormat>(), brush, rc, label_.data(),
                 label_.length());
  }
}

void Item::drawIcon(const gfx::Canvas& gfx) const {
  if (m_iImage < 0)
    return;
  auto const hImageList = GetImageList();
  if (!hImageList)
    return;
  // Note: ILD_TRANSPARENT doesn't effect.
  // Note: ILD_DPISCALE makes background black.
  auto const hIcon = ::ImageList_GetIcon(
      hImageList,
      m_iImage,
      0);
  if (!hIcon)
    return;
  gfx::Bitmap bitmap(gfx, hIcon);
  auto const icon_size = gfx.AlignToPixel(gfx::SizeF(16, 16));
  auto const icon_offset = gfx.AlignToPixel(gfx::SizeF(-20, 8));
  auto const icon_left_top = gfx::PointF(m_rcLabel.left, m_rc.top) +
                             icon_offset;
  gfx->DrawBitmap(bitmap, gfx::RectF(icon_left_top, icon_size));
  ::DestroyIcon(hIcon);
}

bool Item::HasCloseBox() const {
  return IsSelected() || IsHover();
}

bool Item::SetItem(const TCITEM* pTcItem) {
  auto changed = false;
  if (pTcItem->mask & TCIF_IMAGE) {
    if (m_iImage != pTcItem->iImage)
      changed = true;
    m_iImage = pTcItem->iImage;
  }

  if (pTcItem->mask & TCIF_PARAM) {
    m_lParam = pTcItem->lParam;
  }

  if (pTcItem->mask & TCIF_STATE) {
    auto const old_state = m_rgfState;
    m_rgfState &= ~pTcItem->dwStateMask;
    m_rgfState |= pTcItem->dwState & pTcItem->dwStateMask;
    if (m_rgfState != old_state)
      changed = true;
  }

  if (pTcItem->mask & TCIF_TEXT) {
    auto const new_label = pTcItem->pszText;
    if (label_ != new_label) {
      label_ = new_label;
      changed = true;
    }
  }
  return changed;
}

// Element
void Item::Draw(const gfx::Canvas& gfx) const {
  #if DEBUG_HOVER
    DEBUG_PRINTF("%p sel=%d %ls\n",
      this,
      IsSelected(),
      label.c_str());
  #endif

  {
    RECT rc = m_rc;
    gfx::Brush fillBrush(gfx, backgroundColor());
    gfx.FillRectangle(fillBrush, rc);
    gfx::Brush strokeBrush(gfx, gfx::blackColor());
    gfx.DrawRectangle(strokeBrush, rc, 0.2);
  }

  drawContent(gfx);
  if (HasCloseBox())
      m_closeBox.Draw(gfx);
  if (m_rgfState) {
    auto const marker_height = 4;
    auto const marker_width = 4;
    DCHECK_GT(m_rc.right - m_rc.left, marker_width);
    gfx.FillRectangle(
        gfx::Brush(gfx, gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255)),
        gfx::Rect(gfx::Point(m_rc.right - marker_width, m_rc.top),
                  gfx::Size(marker_width, marker_height)));
  }
}

Element* Item::HitTest(POINT pt) const {
  if (HasCloseBox()) {
      if (auto const hit = m_closeBox.HitTest(pt))
        return hit;
  }
  return Element::HitTest(pt);
}

void Item::update() {
  ComputeLayout();
}

//////////////////////////////////////////////////////////////////////
//
// ListButton
//
class ListButton final : public Element {
  DECLARE_CASTABLE_CLASS(ListButton, Element);

  public: ListButton(Element* pParent);
  public: virtual ~ListButton() = default;

  private: void drawDownArrow(const gfx::Canvas& gfx) const;

  // Element
  public: virtual void Draw(const gfx::Canvas& gfx) const override;

  DISALLOW_COPY_AND_ASSIGN(ListButton);
};

ListButton::ListButton(Element* pParent) : Element(pParent) {
}

void ListButton::drawDownArrow(const gfx::Canvas& gfx) const {
  auto const x = (m_rc.right - m_rc.left - 4) / 2 + m_rc.left;
  auto const y = (m_rc.bottom - m_rc.top) / 2 + m_rc.top;
  gfx::Brush arrowBrush(gfx, gfx::blackColor());
  gfx.FillRectangle(arrowBrush, x + 0, y + 0, 5, 1);
  gfx.FillRectangle(arrowBrush, x + 1, y + 1, 3, 1);
  gfx.FillRectangle(arrowBrush, x + 2, y + 2, 1, 1);
}

// Element
void ListButton::Draw(const gfx::Canvas& gfx) const {
  ASSERT(IsShow());
  if (m_rc.left == m_rc.right)
      return;

  gfx::Brush fillBrush(gfx, backgroundColor());
  gfx.FillRectangle(fillBrush, m_rc);
  gfx::Brush strokeBrush(gfx, gfx::blackColor());
  gfx.DrawRectangle(strokeBrush, m_rc, 0.2);

  // Draw triangle
  {
      auto const x = (m_rc.right - m_rc.left - 4) / 2 + m_rc.left;
      auto const y = (m_rc.bottom - m_rc.top) / 2 + m_rc.top;
      fillRect(gfx, x + 0, y + 0, 5, 1);
      fillRect(gfx, x + 1, y + 1, 3, 1);
      fillRect(gfx, x + 2, y + 2, 1, 1);
  }
}


// Cursor for Tab Drag
HCURSOR s_hDragTabCursor;

//////////////////////////////////////////////////////////////////////
//
// loadDragTabCursor
//
// Description:
//  Load cursor for Tab Drag and Drop.
//
static void loadDragTabCursor() {
  if (s_hDragTabCursor) {
    return;
  }

  s_hDragTabCursor = ::LoadCursor(nullptr, IDC_ARROW);

  HMODULE hDll = ::LoadLibraryEx(
    L"ieframe.dll",
    nullptr,
    LOAD_LIBRARY_AS_DATAFILE);
  if (!hDll) {
    return;
  }

  if (auto const hCursor = ::LoadCursor(hDll, MAKEINTRESOURCE(643))) {
    if (auto const hCursor2 = CopyCursor(hCursor)) {
      s_hDragTabCursor = hCursor2;
    }
  }

  ::FreeLibrary(hDll);
}

}  // namespace

namespace views {

//////////////////////////////////////////////////////////////////////
//
// TabStripImpl class

//

// Member variables:
//  m_cxTab
//    Width of tab. m_cxTab and m_xTab are update by computeLayout.
//    We use these variables to avoid redraw.
//  m_xTab
//    Left point of the first tab. m_xTab can be negative if width of
//    Tab Band control is smaller than total number of tabs times
//    m_cxTab.
//
class TabStrip::TabStripImpl final : public Element {
  DECLARE_CASTABLE_CLASS(TabStripImpl, Element);

  friend class TabStrip;

  private: enum Constants {
    k_TabListId = 1000,
    k_TabViewId,
    k_ScrollLeft,
    k_ScrollRight,
  };

  private: enum Drag {
    Drag_None,

    Drag_Tab,
    Drag_Start,
  };

  private: typedef std::vector<Element*> Elements;

  private: gfx::Canvas m_gfx;
  private: int num_tab_items_;
  private: BOOL m_compositionEnabled;
  private: int m_cxTab;
  private: int m_cxMinTab;
  private: TabStripDelegate* delegate_;
  private: Drag m_eDrag;
  private: HMENU m_hTabListMenu;
  private: HWND m_hwnd;
  private: HWND m_hwndToolTips;
  private: int m_iFocus;
  private: uint m_nStyle;
  private: ListButton m_oListButton;
  private: Elements m_oElements;
  private: Item* m_pDragItem;
  private: Element* m_pHover;
  private: Item* m_pInsertBefore;
  private: Item* m_pSelected;
  private: POINT m_ptDragStart;
  private: int m_xTab;
  private: base::string16 tooltip_text_;

  public: TabStripImpl(HWND hwnd, TabStripDelegate* delegate);
  public: virtual ~TabStripImpl();

  // [C]
  private: bool changeFont(const gfx::Canvas& canvas);

  // [D]
  private: void DidChangeTabSelection();
  public: void DidCreateNativeWindow();
  public: void DeleteTab(int iDeleteItem);
  private: void DropTab(Item* item, const POINT& point);

  // [F]
  private: Elements::iterator FindItem(int item_index);

  // [G]
  private: Item* GetTabFromIndex(int iItem) const;
  public: bool GetTab(int tab_index, TCITEM* pTcItem) const;

  // [H]
  private: void handleTabListMenu(POINT point);
  private: Element* hitTest(POINT point) const;

  // [I]
  private: void InsertTab(int iItem, const TCITEM* pTcItem);

  // [O]
  private: void onLButtonDown(POINT pt);
  private: void onLButtonUp(POINT pt);
  private: LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  private: void OnMouseMove(POINT pt);
  private: LRESULT OnNotify(NMHDR* nmhdr);

  // [R]
  public: void Redraw();
  private: void RenumberTabIndex();

  // [S]
  private: int SelectItem(int const iItem);
  private: int SelectItem(Item* const pItem);
  private: void stopDrag();

  // [U]
  private: void UpdateHover(Element* pHover);
  private: bool UpdateLayout();

  // Element
  public: void Draw(const gfx::Canvas& gfx) const override;

  DISALLOW_COPY_AND_ASSIGN(TabStripImpl);
};

TabStrip::TabStripImpl::TabStripImpl(HWND hwnd, TabStripDelegate* delegate)
    : Element(nullptr),
      num_tab_items_(0),
      m_compositionEnabled(false),
      m_cxTab(0),
      m_cxMinTab(k_cxMinTab),
      delegate_(delegate),
      m_eDrag(Drag_None),
      m_hTabListMenu(nullptr),
      m_hwnd(hwnd),
      m_hwndToolTips(nullptr),
      m_iFocus(-1),
      m_nStyle(0),
      m_oListButton(this),
      m_pDragItem(nullptr),
      m_pHover(nullptr),
      m_pInsertBefore(nullptr),
      m_pSelected(nullptr),
      m_xTab(0) {
  m_oElements.push_back(&m_oListButton);
  COM_VERIFY(::DwmIsCompositionEnabled(&m_compositionEnabled));
}

TabStrip::TabStripImpl::~TabStripImpl() {
  if (auto const text_format = m_gfx.work<gfx::TextFormat>())
      delete text_format;

  if (m_hwndToolTips && (m_nStyle & TCS_TOOLTIPS) != 0) {
    ::DestroyWindow(m_hwndToolTips);
  }

  if (m_hTabListMenu) {
    ::DestroyMenu(m_hTabListMenu);
  }
}

bool TabStrip::TabStripImpl::changeFont(const gfx::Canvas& gfx) {
  LOGFONT lf;
  if (!::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
    return false;

  //lf.lfHeight = -13;

  if (auto const old_format = gfx.work<gfx::TextFormat*>())
      delete old_format;
  gfx.set_work(new gfx::TextFormat(lf));
  return true;
}

void TabStrip::TabStripImpl::DeleteTab(int iDeleteItem) {
  auto present = FindItem(iDeleteItem);
  if (present == m_oElements.end())
    return;

  auto const selection_changed = m_pSelected == *present;
  if (selection_changed)
    m_pSelected = GetTabFromIndex(iDeleteItem ? iDeleteItem - 1 : 1);

  if (m_pHover == *present)
    m_pHover = nullptr;

  m_oElements.erase(present);
  RenumberTabIndex();

  if (m_hwndToolTips) {
    TOOLINFO ti;
    ti.cbSize = sizeof(ti);
    ti.hwnd = m_hwnd;
    ti.uId = static_cast<DWORD>(num_tab_items_);
    ::SendMessage(
        m_hwndToolTips,
        TTM_DELTOOL,
        0,
        reinterpret_cast<LPARAM>(&ti));
  }

  Redraw();

  if (selection_changed) {
    if (m_pSelected)
      m_pSelected->SetState(Element::State_Selected);
    DidChangeTabSelection();
  }
}

void TabStrip::TabStripImpl::DidChangeTabSelection() {
  delegate_->DidChangeTabSelection(m_pSelected ? m_pSelected->tab_index() : -1);
}

void TabStrip::TabStripImpl::DidCreateNativeWindow() {
  m_gfx.Init(m_hwnd);
  changeFont(m_gfx);

  m_hwndToolTips = ::CreateWindowEx(
      WS_EX_TOPMOST,
      TOOLTIPS_CLASS,
      nullptr,
      WS_POPUP | TTS_NOPREFIX, // | TTS_ALWAYSTIP,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      m_hwnd,
      nullptr,
      g_hInstance,
      nullptr);

  if (m_hwndToolTips) {
    m_nStyle |= TCS_TOOLTIPS;

    TOOLINFO ti;
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hwnd;
    ti.uId = reinterpret_cast<UINT_PTR>(m_hwnd);
    ti.lpszText = nullptr;
    ::SendMessage(
        m_hwndToolTips,
        TTM_ADDTOOL,
        0,
        reinterpret_cast<LPARAM>(&ti));
  }
}

// Send TabDragMsg to window which can handle it.
void TabStrip::TabStripImpl::DropTab(Item* item, const POINT& ptClient) {
  auto ptScreen = ptClient;
  if (!::ClientToScreen(m_hwnd, &ptScreen))
    return;

  for (auto hwnd = ::WindowFromPoint(ptScreen); hwnd;
       hwnd = ::GetParent(hwnd)) {
    if (auto const frame = FrameList::instance()->FindFrameByHwnd(hwnd)) {
      static_cast<TabStripDelegate*>(frame)->OnDropTab(item->m_lParam);
      return;
    }
  }

  delegate_->DidThrowTab(item->m_lParam);
}

TabStrip::TabStripImpl::Elements::iterator TabStrip::TabStripImpl::FindItem(
    int tab_index) {
  for (auto it = m_oElements.begin(); it != m_oElements.end(); ++it) {
    auto const tab_item = (*it)->as<Item>();
    if (tab_item && tab_item->tab_index() == tab_index)
      return it;
  }
  return m_oElements.end();
}

bool TabStrip::TabStripImpl::GetTab(int tab_index, TCITEM* pTcItem) const {
  auto const tab_item = GetTabFromIndex(tab_index);
  if (!tab_item)
    return false;

  if (pTcItem->mask & TCIF_IMAGE) {
    pTcItem->iImage = tab_item->m_iImage;
  }

  if (pTcItem->mask & TCIF_PARAM) {
    pTcItem->lParam = tab_item->m_lParam;
  }

  if (pTcItem->mask & TCIF_STATE) {
    pTcItem->dwState = tab_item->m_rgfState & pTcItem->dwStateMask;
  }

  if (pTcItem->mask & TCIF_TEXT) {
    auto const cwch = std::min(
      tab_item->label_.length(),
      static_cast<size_t>(pTcItem->cchTextMax - 1));

    ::CopyMemory(
        pTcItem->pszText,
        tab_item->label_.data(),
        sizeof(base::char16) * cwch);

    pTcItem->pszText[cwch] = 0;
  }
  return true;
}

Item* TabStrip::TabStripImpl::GetTabFromIndex(int tab_index) const {
  for (auto element : m_oElements) {
    auto const tab_item = element->as<Item>();
    if (tab_item && tab_item->tab_index() == tab_index)
      return tab_item;
  }
  return nullptr;
}

void TabStrip::TabStripImpl::handleTabListMenu(POINT) {
  POINT ptMouse;
  ptMouse.x = m_oListButton.GetRect()->left;
  ptMouse.y = m_oListButton.GetRect()->bottom;

  ::ClientToScreen(m_hwnd, &ptMouse);

  if (!m_hTabListMenu)
    m_hTabListMenu = ::CreatePopupMenu();

  // Make Tab List Menu empty
  while (::GetMenuItemCount(m_hTabListMenu) > 0) {
    ::DeleteMenu(m_hTabListMenu, 0, MF_BYPOSITION);
  }

  // Add Tab name to menu.
  Item* pPrevItem = nullptr;
  for (auto element : m_oElements) {
    auto const pItem = element->as<Item>();
    if (!pItem)
      continue;

    auto const rgfFlag = pItem->IsSelected() ? MF_STRING | MF_CHECKED :
                                               MF_STRING;

    if (pPrevItem && pPrevItem->IsShow() != pItem->IsShow()) {
      ::AppendMenu(
          m_hTabListMenu,
          MF_SEPARATOR,
          0,
          nullptr);
    }

    pPrevItem = pItem;

    ::AppendMenu(
        m_hTabListMenu,
        static_cast<DWORD>(rgfFlag),
        static_cast<DWORD>(pItem->tab_index()),
        pItem->label_.c_str());
  }

  ::TrackPopupMenuEx(
      m_hTabListMenu,
      TPM_LEFTALIGN | TPM_TOPALIGN,
      ptMouse.x, ptMouse.y,
      m_hwnd,
      nullptr);
}

Element* TabStrip::TabStripImpl::hitTest(POINT pt) const {
  if (auto const pHit = m_oListButton.HitTest(pt))
    return pHit;

  for (auto element : m_oElements) {
    auto const tab_item = element->as<Item>();
    if (!tab_item)
      continue;

    if (pt.x < tab_item->GetRect()->left)
      break;

    if (auto const hit = tab_item->HitTest(pt))
      return hit;
  }

  return nullptr;
}

void TabStrip::TabStripImpl::InsertTab(int tab_index, const TCITEM* pTcItem) {
  auto const new_tab_item = new Item(this, pTcItem);
  auto present = FindItem(tab_index);
  if (present == m_oElements.end()) {
    m_oElements.push_back(new_tab_item);
  } else {
    if (m_iFocus >= tab_index)
      ++m_iFocus;
    m_oElements.insert(present, new_tab_item);
  }
  ++num_tab_items_;
  RenumberTabIndex();

  if (m_hwndToolTips) {
    TOOLINFO ti;
    ti.cbSize = sizeof(ti);
    ti.hwnd = m_hwnd;
    ti.lpszText = LPSTR_TEXTCALLBACK;
    ti.uFlags = 0;
    ti.uId = static_cast<DWORD>(num_tab_items_ - 1);
    ::SendMessage(m_hwndToolTips, TTM_ADDTOOL, 0,
                  reinterpret_cast<LPARAM>(&ti));
  }

  Redraw();
}

void TabStrip::TabStripImpl::onLButtonDown(POINT pt) {
  auto const pElement = hitTest(pt);
  if (!pElement) {
    return;
  }

  auto const pItem = pElement->as<Item>();
  if (!pItem) {
    // Not a tab.
    return;
  }

  if (!pItem->IsSelected()) {
    SelectItem(pItem);
    // Note: We should start tab dragging, otherwise if
    // mouse pointer is in close box, onButtonUp close
    // the tab.
  }

  #if DEBUG_DRAG
    DEBUG_PRINTF("%p drag=%p\n", this, m_pDragItem);
  #endif

  loadDragTabCursor();

  m_pDragItem = pItem;
  m_eDrag = Drag_Start;
  m_ptDragStart = pt;

  ::SetCapture(m_hwnd);
}

void TabStrip::TabStripImpl::onLButtonUp(POINT pt) {
  if (!m_pDragItem) {
    auto const element = hitTest(pt);
    if (!element)
      return;

    if (element->is<CloseBox>()) {
      if (auto const item = element->GetParent()->as<Item>())
        delegate_->DidClickTabCloseButton(item->tab_index());
      return;
    }

    if (element->is<ListButton>()) {
      handleTabListMenu(pt);
      return;
    }

    return;
  }

  auto const pDragItem = m_pDragItem;
  auto const pInsertBefore = m_pInsertBefore;
  stopDrag();

  if (!pInsertBefore) {
    DropTab(pDragItem, pt);
    return;
  }

  if (pDragItem != pInsertBefore) {
    m_oElements.erase(std::find(m_oElements.begin(), m_oElements.end(),
                                pDragItem));
    m_oElements.insert(std::find(m_oElements.begin(), m_oElements.end(),
                                 pInsertBefore),
                       pDragItem);
    RenumberTabIndex();
    UpdateLayout();
  }

  // Hide insertion position mark
  ::InvalidateRect(m_hwnd, nullptr, false);
}

LRESULT TabStrip::TabStripImpl::OnMessage(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  static UINT last_message = 0;
  #if DEBUG_MESSAGE
    if (uMsg != 0x133C && last_message != uMsg) {
      last_message = uMsg;
      DVLOG(0) << "TabStrip::TabBand " << this << " msg=" << std::hex <<
          uMsg;
    }
  #endif

  switch (uMsg) {
    case WM_DWMCOMPOSITIONCHANGED:
      if (FAILED(DwmIsCompositionEnabled(&m_compositionEnabled)))
          m_compositionEnabled = false;
      break;

    case WM_NCHITTEST: {
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      if (::ScreenToClient(m_hwnd, &pt)) {
        if (!hitTest(pt))
          return ::SendMessage(::GetParent(m_hwnd), uMsg, wParam, lParam);
        return HTCLIENT;
      }
      return HTNOWHERE;
    }

    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_NCMOUSEMOVE:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
      // Redirect non-client mouse move to parent for top level window
      // management, e.g. moving top level window by grabing empty area
      // of tabs.
      return ::SendMessage(::GetParent(m_hwnd), uMsg, wParam, lParam);

    case WM_SETTINGCHANGE:
      switch (wParam) {
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETNONCLIENTMETRICS: {
          changeFont(m_gfx);
          break;
        }
      }
      break;
  }

  return 0;
}

void TabStrip::TabStripImpl::OnMouseMove(POINT pt) {
  auto const pHover = hitTest(pt);

  if (!m_pDragItem) {
    UpdateHover(pHover);
  } else {
    if (::GetCapture() != m_hwnd) {
      // Someone takes capture. So, we stop dragging.
      stopDrag();
      return;
    }

    if (Drag_Start == m_eDrag) {
      if (pt.x - m_ptDragStart.x >= -5 &&
          pt.x - m_ptDragStart.x <= 5) {
        return;
      }

      m_eDrag = Drag_Tab;
    }

    // Tab dragging
    auto const pInsertBefore = pHover ? pHover->as<Item>() : nullptr;
    ::SetCursor(s_hDragTabCursor);
    if (pInsertBefore != m_pInsertBefore)
      ::InvalidateRect(m_hwnd, nullptr, false);
    m_pInsertBefore = pInsertBefore;
  }
}

LRESULT TabStrip::TabStripImpl::OnNotify(NMHDR* nmhdr) {
  if (nmhdr->hwndFrom != m_hwndToolTips)
    return 0;
  if (nmhdr->code != TTN_NEEDTEXT)
    return 0;
  // Set width of tooltip
  ::SendMessage(m_hwndToolTips, TTM_SETMAXTIPWIDTH, 0, 300);
  auto const disp_info = reinterpret_cast<NMTTDISPINFO*>(nmhdr);
  auto const tab_index = static_cast<int>(nmhdr->idFrom);
  tooltip_text_ = delegate_->GetTooltipTextForTab(tab_index);
  disp_info->lpszText = const_cast<LPWSTR>(tooltip_text_.c_str());
  return 0;
}

void TabStrip::TabStripImpl::Redraw() {
  UpdateLayout();
  ::InvalidateRect(m_hwnd, nullptr, false);
}

void TabStrip::TabStripImpl::RenumberTabIndex() {
  auto tab_index = 0;
  for (auto element : m_oElements) {
    if (auto const tab_item = element->as<Item>()) {
      tab_item->set_tab_index(tab_index);
      ++tab_index;
    }
  }
}

int TabStrip::TabStripImpl::SelectItem(int const iItem) {
  return SelectItem(GetTabFromIndex(iItem));
}

int TabStrip::TabStripImpl::SelectItem(Item* const pItem) {
  if (m_pSelected != pItem) {
    if (m_pSelected) {
      m_pSelected->SetState(Element::State_Normal);
      m_pSelected->Invalidate(m_hwnd);
    }

    m_pSelected = pItem;

    if (pItem) {
      pItem->SetState(Element::State_Selected);
      if (!pItem->IsShow())
        Redraw();
      pItem->Invalidate(m_hwnd);
    }

    DidChangeTabSelection();
  }

  return m_pSelected ? m_pSelected->tab_index() : -1;
}

void TabStrip::TabStripImpl::stopDrag() {
  m_eDrag = Drag_None;
  m_pDragItem = nullptr;
  m_pInsertBefore = nullptr;

  ::ReleaseCapture();
  ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

void TabStrip::TabStripImpl::UpdateHover(Element* pHover) {
    if (m_pHover == pHover)
      return;

  if (m_pHover) {
    if (!pHover || !pHover->is<CloseBox>() ||
        pHover->GetParent() != m_pHover) {
      if (m_pHover->is<CloseBox>()) {
        m_pHover->SetHover(false);
        m_pHover = m_pHover->GetParent();
      }
      m_pHover->SetHover(false);
      m_pHover->Invalidate(m_hwnd);
    }
  }

  m_pHover = pHover;
  if (m_pHover) {
    m_pHover->SetHover(true);
    m_pHover->Invalidate(m_hwnd);
  }
}

bool TabStrip::TabStripImpl::UpdateLayout() {
  if (!num_tab_items_) {
    m_cxTab = -1;
    m_xTab = -1;
    return false;
  }

  *m_oListButton.GetRect() = m_rc;

  m_oListButton.GetRect()->left = m_rc.left + k_cxMargin;

  auto x = m_oListButton.GetRect()->left;

  if (num_tab_items_ >= 2) {
    m_oListButton.Show(true);
    x += k_cxListButton;
  } else {
    m_oListButton.Show(false);
  }

  m_oListButton.GetRect()->right = x;

  int cxTabs = m_rc.right - x - k_cxMargin;

  int cxTab = cxTabs / num_tab_items_;
    cxTab = std::min(cxTab, m_cxMinTab * 2);

  if (cxTab >= m_cxMinTab) {
    // We can show all tabs.
  } else {
    // How many tabs do we show in min width?
    int cVisibles = cxTabs / m_cxMinTab;
    if (cVisibles == 0) {
      cVisibles = 1;
    }
    cxTab = cxTabs / cVisibles;
  }

  bool fChanged = m_cxTab != cxTab || m_xTab != x;

  m_cxTab = cxTab;
  m_xTab = x;

  for (auto view_start_tab_index = 0; view_start_tab_index < num_tab_items_;
       ++view_start_tab_index) {
    auto fShow = false;
    x = m_xTab;
    for (auto element : m_oElements) {
      auto const tab_item = element->as<Item>();
      if (!tab_item)
        continue;

      if (tab_item->tab_index() == view_start_tab_index)
        fShow = true;

      tab_item->Show(fShow);

      if (!fShow)
        continue;

      RECT* prc = tab_item->GetRect();
      prc->left = x;
      prc->right = x + cxTab;
      prc->top = m_rc.top;
      prc->bottom = m_rc.bottom;
      tab_item->ComputeLayout();

      x += cxTab;

      fShow = x + cxTab < m_rc.right;
    }

    if (!m_pSelected || m_pSelected->IsShow())
      break;
  }

  if (m_hwndToolTips) {
    TOOLINFO ti;
    ti.cbSize = sizeof(ti);
    ti.hwnd = m_hwnd;
    ti.uFlags = 0;

    for (auto element : m_oElements) {
      if (element->is<Item>()) {
        ti.uId = static_cast<DWORD>(element->as<Item>()->tab_index());
      } else if (element->is<ListButton>()) {
        ti.uId = k_TabListId;
      } else {
        continue;
      }

      if (element->IsShow()) {
        ti.rect = *element->GetRect();
      } else {
        ti.rect.left = ti.rect.right = -1;
        ti.rect.top = ti.rect.bottom = -1;
      }

      ::SendMessage(
          m_hwndToolTips,
          TTM_NEWTOOLRECT,
          0,
          reinterpret_cast<LPARAM>(&ti));
    }
  }

  return fChanged;
}

// Element
void TabStrip::TabStripImpl::Draw(const gfx::Canvas& gfx) const {
  struct Local {
    static void drawInsertMarker(const gfx::Canvas& gfx, RECT* prc) {
      auto rc = * prc;
      rc.top += 5;
      rc.bottom -= 7;

      for (int w = 1; w <= 7; w += 2) {
        fillRect(gfx, rc.left, rc.top, w, 1);
        fillRect(gfx, rc.left, rc.bottom, w, 1);

        rc.top  -= 1;
        rc.left   -= 1;
        rc.bottom += 1;
      }
    }
  };

  gfx->SetTransform(D2D1::IdentityMatrix());
  gfx->Clear(gfx::sysColor(COLOR_3DFACE,
                           m_compositionEnabled ? 0.0f : 1.0f));

  for (auto element : m_oElements) {
    if (element->IsShow())
      element->Draw(gfx);
  }

  if (m_pInsertBefore)
    Local::drawInsertMarker(m_gfx, m_pInsertBefore->GetRect());
}

//////////////////////////////////////////////////////////////////////
//
// TabStrip
//
TabStrip::TabStrip(TabStripDelegate* delegate)
    : ui::Widget(ui::NativeWindow::Create(this)), delegate_(delegate) {
}

TabStrip::~TabStrip() {
}

int TabStrip::number_of_tabs() const {
  return impl_->num_tab_items_;
}

int TabStrip::selected_index() const {
  return impl_->m_pSelected ? impl_->m_pSelected->tab_index() : -1;
}

void TabStrip::DeleteTab(int tab_index) {
  impl_->DeleteTab(tab_index);
}

Size TabStrip::GetPreferreSize() const {
  auto const font_height = 16;  // must be >= 16 (Small Icon Height)
  return Size(font_height * 40, 2 + 7 + font_height + 5 + 2);
}

bool TabStrip::GetTab(int tab_index, TCITEM* tab_data) {
  return impl_->GetTab(tab_index, tab_data);
}

void TabStrip::InsertTab(int new_tab_index, const TCITEM* tab_data) {
  impl_->InsertTab(new_tab_index, tab_data);
}

void TabStrip::SelectTab(int tab_index) {
  impl_->SelectItem(tab_index);
}

void TabStrip::SetIconList(HIMAGELIST icon_list) {
  impl_->SetImageList(icon_list);
  impl_->Redraw();
}

void TabStrip::SetTab(int tab_index, const TCITEM* tab_data) {
  auto const item = impl_->GetTabFromIndex(tab_index);
  if (!item)
    return;
  if (item->SetItem(tab_data))
    item->Invalidate(impl_->m_hwnd);
}

// ui::Widget
void TabStrip::CreateNativeWindow() const {
  native_window()->CreateWindowEx(
      0, WS_CHILD | WS_VISIBLE, L"TabStrip", parent_node()->AssociatedHwnd(),
      bounds().left_top(),
      bounds().size());
}

void TabStrip::DidCreateNativeWindow() {
  impl_.reset(new TabStripImpl(*native_window(), delegate_));
  impl_->m_rc = bounds();
  impl_->DidCreateNativeWindow();
}

void TabStrip::DidResize() {
  if (!impl_)
    return;
  impl_->m_rc = bounds();
  impl_->m_gfx.Resize(bounds());
  impl_->Redraw();
}

LRESULT TabStrip::OnMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_COMMAND:
      SelectTab(static_cast<int>(LOWORD(wParam)));
      return 0;
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_NCHITTEST:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_NCMOUSEMOVE:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
    case WM_SETTINGCHANGE:
    case WM_USER:
      return impl_->OnMessage(uMsg, wParam, lParam);

    case WM_NOTIFY:
      return impl_->OnNotify(reinterpret_cast<NMHDR*>(lParam));
  }

  return ui::Widget::OnMessage(uMsg, wParam, lParam);
}

void TabStrip::OnMouseExited(const ui::MouseEvent&) {
  impl_->UpdateHover(nullptr);
}

void TabStrip::OnMouseMoved(const ui::MouseEvent& event) {
  impl_->OnMouseMove(event.location());
}

void TabStrip::OnMousePressed(const ui::MouseEvent& event) {
  if (event.is_left_button() && !event.click_count())
    impl_->onLButtonDown(event.location());
}

void TabStrip::OnMouseReleased(const ui::MouseEvent& event) {
  if (event.is_left_button())
    impl_->onLButtonUp(event.location());
}

void TabStrip::OnPaint(const Rect rect) {
  gfx::Canvas::DrawingScope drawing_scope(impl_->m_gfx);
  impl_->m_gfx.set_dirty_rect(rect);
  impl_->Draw(impl_->m_gfx);
}

}  // namespace views
