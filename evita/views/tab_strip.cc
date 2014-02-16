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

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

namespace {

#define WC_TABBANDCLASS  L"TabBandClass"

static HINSTANCE g_hInstance;

template<class Item_>
class DoubleLinkedList_;

template<class Item_>
class DoubleLinkedNode_ {
  friend class DoubleLinkedList_<Item_>;

  private: Item_* m_pNext;
  private: Item_* m_pPrev;

  public: DoubleLinkedNode_() :
    m_pNext(nullptr),
    m_pPrev(nullptr) {}

  public: Item_* GetNext() const { return m_pNext; }
  public: Item_* GetPrev() const { return m_pPrev; }
}; // DoubleLinkedNode_

template<class Item_>
class DoubleLinkedList_ {
  protected: typedef DoubleLinkedList_<Item_> List_;

  private: Item_* m_pFirst;
  private: Item_* m_pLast;

  public: DoubleLinkedList_()
    : m_pFirst(nullptr),
      m_pLast(nullptr) {}

  // [A]
  public: Item_* Append(Item_* pItem) {
    pItem->m_pNext = nullptr;
    pItem->m_pPrev = m_pLast;

    if (!m_pFirst) {
      m_pFirst = pItem;
    }

    if (m_pLast) {
      m_pLast->m_pNext = pItem;
    }

    return m_pLast = pItem;
  }

  // [D]
  public: Item_* Delete(Item_* pItem) {
    auto const pNext = pItem->m_pNext;
    auto const pPrev = pItem->m_pPrev;
    if (!pNext) {
      m_pLast = pPrev;
    } else {
      pNext->m_pPrev = pPrev;
    }

    if (!pPrev) {
      m_pFirst = pNext;
    } else {
      pPrev->m_pNext = pNext;
    }

    pItem->m_pNext = nullptr;
    pItem->m_pPrev = nullptr;

    return pItem;
  }

  // [E]
  public: class Enum {
    private: Item_* m_pRunner;
    public: Enum(List_* p) : m_pRunner(p->m_pFirst) {}
    public: Enum(const List_* p) : m_pRunner(p->m_pFirst) {}
    public: bool AtEnd() const { return m_pRunner == nullptr; }
    public: Item_* Get() { return m_pRunner; }

    public: void Next() {
      ASSERT(!AtEnd());
      m_pRunner = m_pRunner->m_pNext;
    }
  }; // Enum

  // [G]
  public: Item_* GetFirst() const { return m_pFirst; }
  public: Item_* GetLast()  const { return m_pLast; }

  // [I]
  public: Item_* InsertBefore(Item_* pItem, Item_* pRefItem) {
    if (!pRefItem) {
      return Append(pItem);
    }

    auto const pPrev = pRefItem->m_pPrev;
    if (!pPrev) {
      m_pFirst = pItem;
    } else {
      pPrev->m_pNext = pItem;
    }

    pItem->m_pPrev = pPrev;
    pItem->m_pNext = pRefItem;

    pRefItem->m_pPrev = pItem;
    return pItem;
  }

  public: bool IsEmpty() const {
    return m_pFirst == nullptr;
  }
}; // DoubleLinkedList_

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element : public DoubleLinkedNode_<Element> {
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
  protected: Element(Element* pParent)
      : m_eState(State_Normal)
      , m_fHover(false)
      , m_fShow(true)
      , m_hImageList(nullptr)
      , m_pParent(pParent) {
    m_rc.left = m_rc.top = m_rc.right = m_rc.top = 0;
  }

  public: virtual ~Element() {
  }

  protected: gfx::ColorF backgroundColor() const {
    if (IsSelected())
        return gfx::whiteColor();
    if (IsHover())
        return gfx::sysColor(COLOR_3DHILIGHT, 0.8);
    return gfx::sysColor(COLOR_3DFACE, 0.5);
  }

  // [D]
  public: virtual void Draw(const gfx::Graphics&) const = 0;

  protected: static void fillRect(const gfx::Graphics& gfx, int x, int y,
                                  int cx, int cy) {
    RECT rc;
    rc.left = x;
    rc.right = x + cx;
    rc.top = y;
    rc.bottom = y + cy;
    gfx::Brush brush(gfx, gfx::blackColor());
    gfx.FillRectangle(brush, rc);
  }

  public: template<class T> T* DynamicCast() {
    return Is<T>() ? StaticCast<T>() : nullptr;
  }

  // [G]
  public: virtual const char16* GetClass() const = 0;

  public: bool GetHover() const { return m_fHover; }

  public: HIMAGELIST GetImageList() const {
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

  public: Element* GetNextShow() const {
    for (
        Element* pRunner = GetNext();
        pRunner;
        pRunner = pRunner->GetNext()) {
      if (pRunner->m_fShow) {
        return pRunner;
      }
    }
    return nullptr;
  }

  public: Element* GetPrevShow() const {
    for (
        Element* pRunner = GetPrev();
        nullptr != pRunner;
        pRunner = pRunner->GetPrev()) {
      if (pRunner->m_fShow) {
        return pRunner;
      }
    }
    return nullptr;
  }

  public: RECT*  GetRect() { return &m_rc; }
  public: Element* GetParent() const { return m_pParent; }
  public: State  GetState() const { return m_eState; }

  // [H]
  public: virtual Element* HitTest(POINT pt) const {
    if (!::PtInRect(&m_rc, pt)) {
      return nullptr;
    }

    return const_cast<Element*>(this);
  }

  // [I]
  public: void Invalidate(HWND hwnd)
    {  ::InvalidateRect(hwnd, &m_rc, false); }

  public: template<class T> bool Is() const
    { return T::GetClass_() == GetClass(); }

  // IsHover
  public: bool IsHover() const {
    return m_fHover;
  }

  // IsSelected
  public: bool IsSelected() const {
    return State_Selected == m_eState;
  }

  public: bool IsShow() const {
    return m_fShow;
  }

  // [S]
  public: bool SetHover(bool f) {
    return m_fHover = f;
  }

  public: HIMAGELIST SetImageList(HIMAGELIST h) {
    return m_hImageList = h;
  }

  public: Element* SetParent(Element* p) {
    return m_pParent = p;
  }

  public: State SetState(State e) {
    m_eState = e;
    update();
    return m_eState;
  }

  public: bool Show(bool f) {
    return m_fShow = f;
  }

  public: template<class T> T* StaticCast() {
    ASSERT(Is<T>()); 
    // warning C4946: reinterpret_cast used between related classes: 
    // 'class1' and 'class2'
    #pragma warning(suppress: 4946)
    return reinterpret_cast<T*>(this);
  }

  // [U]
  protected: virtual void update() {}
}; // Element

//////////////////////////////////////////////////////////////////////
//
// TabBand Design Parameters
//
enum TabBandDesignParams {
  k_cxMargin = 3,
  k_cyMargin = 2,
  k_cxListButton = 16,
  k_cxEdge = 2,
  k_cxBorder = 3,
  k_cxPad = 3,
  k_cxMinTab = 140,
  k_cyBorder = 5,
  k_cyIcon = 16,
}; // TabBandDesignParams

//////////////////////////////////////////////////////////////////////
//
// CloseBox
//
class CloseBox : public Element {
  public: static const char16*  GetClass_() { return L"CloseBox"; }

  public: virtual const char16* GetClass() const override {
    return GetClass_();
  }

  public: enum Design {
    Height = 16,
    Width = 17,
  }; // Desgin

  // ctor
  public: CloseBox(Element* pParent) :
    Element(pParent) {}

  // [D]

  private: void drawXMark(const gfx::Graphics& gfx, gfx::ColorF color) const {
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

  public: virtual void Draw(const gfx::Graphics& gfx) const override {
    drawXMark(gfx, markColor());
  }

  private: gfx::ColorF markColor() const {
    return IsHover() ? gfx::ColorF::DarkViolet : gfx::ColorF::DimGray;
  }
}; // CloseBox

//////////////////////////////////////////////////////////////////////
//
// Item
//  Represents tab item.
//
class Item : public Element {
  private: enum Design {
    k_cxCloseBoxMargin = 3,
    k_cyCloseBoxMargin = 9,
    k_cyDescent = 4,
  }; // Design

  public: static const char16*  GetClass_() { return L"Item"; }
  public: virtual const char16* GetClass()  const { return GetClass_(); }

  public: base::string16 label_;
  public: int m_iItem;
  public: int m_iImage;
  public: LPARAM m_lParam;
  private: RECT m_rcLabel;
  private: CloseBox m_closeBox;
  public: uint m_rgfState;

  // ctor
  public: Item(Element* pParent, int iItem, const TCITEM* pTcItem) :
      m_iImage(-1),
      m_iItem(iItem),
      m_rgfState(0),
      m_closeBox(this),
      Element(pParent) {
    SetItem(pTcItem);
  }

  public: ~Item() = default;

  public: void ComputeLayout() {
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

  // [D]
  // Draw
  public: virtual void Draw(const gfx::Graphics& gfx) const override {
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
  }

  private: void drawContent(const gfx::Graphics& gfx) const {
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

  private: void drawIcon(const gfx::Graphics& gfx) const {
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
  }

  // [H]
  public: bool HasCloseBox() const {
    return IsSelected() || IsHover();
  }

  public: virtual Element* HitTest(POINT pt) const override {
    if (HasCloseBox()) {
        if (auto const hit = m_closeBox.HitTest(pt))
          return hit;
    }
    return Element::HitTest(pt);
  }

  // [S]
  public: bool SetItem(const TCITEM* pTcItem) {
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
      m_rgfState |= pTcItem->dwState | pTcItem->dwStateMask;
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

  // [U]
  protected: void update() override {
    ComputeLayout();
  }
}; // Item

//////////////////////////////////////////////////////////////////////
//
// ListButton
//
//
//    [4]    [5]
//    --oooooooooo--
//    -o----------o-
// [3]o------------o [6]
//    o------------o
//    o------------o
//    o------------o
//    o------------o
//    o-----------o- [0]
//    oooooooooooo--
//  [2]           [1]
//
class ListButton : public Element {
  public: static  const char16*  GetClass_() { return L"ListButton"; }

  public: virtual const char16* GetClass()  const override {
    return GetClass_();
  }

  // ctor
  public: ListButton(Element* pParent) :
    Element(pParent) {}

  // [D]
  public: virtual void Draw(const gfx::Graphics& gfx) const override {
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

  private: void drawDownArrow(const gfx::Graphics& gfx) const {
    auto const x = (m_rc.right - m_rc.left - 4) / 2 + m_rc.left;
    auto const y = (m_rc.bottom - m_rc.top) / 2 + m_rc.top;
    gfx::Brush arrowBrush(gfx, gfx::blackColor());
    gfx.FillRectangle(arrowBrush, x + 0, y + 0, 5, 1);
    gfx.FillRectangle(arrowBrush, x + 1, y + 1, 3, 1);
    gfx.FillRectangle(arrowBrush, x + 2, y + 2, 1, 1);
  }
}; // ListButton

//////////////////////////////////////////////////////////////////////
//
// Cursor for Tab Drag
//
static HCURSOR s_hDragTabCursor;

// Window Message for Tab Dragging.
static uint s_nTagDragMsg;

// Send TabDragMsg to window which can handle it.
static HWND handleTabDragAndDrop(
    HWND const hwndTabBand,
    POINT const ptClient,
    TabBandDragAndDrop const eAction) {
  auto ptScreen = ptClient;
  if (!::ClientToScreen(hwndTabBand, &ptScreen)) {
    return nullptr;
  }

  auto hwnd = ::WindowFromPoint(ptScreen);
  if (!hwnd) {
    return nullptr;
  }

  if (s_nTagDragMsg == 0) {
    s_nTagDragMsg = ::RegisterWindowMessage(TabBand__TabDragMsgStr);
    if (s_nTagDragMsg == 0) {
      DEBUG_PRINTF("Failed RegisterWindowMessage %ls\n",
        TabBand__TabDragMsgStr);
      return nullptr;
    }
  }

  do {
    auto const iAnswer = ::SendMessage(
      hwnd,
      s_nTagDragMsg,
      eAction,
      reinterpret_cast<LPARAM>(hwndTabBand));

    if (iAnswer) {
      return hwnd;
    }

    hwnd = ::GetParent(hwnd);
  } while (hwnd);

  if (eAction == kDrop) {
    auto const hwnd = ::GetParent(hwndTabBand);
    auto const iAnswer = ::SendMessage(
        hwnd,
        s_nTagDragMsg,
        kThrow,
        reinterpret_cast<LPARAM>(hwndTabBand));
    return iAnswer ? hwnd : nullptr;
  }

  return nullptr;
}

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

//////////////////////////////////////////////////////////////////////
//
// TabBand class

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
class TabBand : public Element {
  private: enum Constants {
    k_TabListId = 1000,
    k_TabViewId,
    k_ScrollLeft,
    k_ScrollRight,
  }; // Constatns

  private: enum Drag {
    Drag_None,

    Drag_Tab,
    Drag_Start,
  }; // Drag

  public: static const char16*  GetClass_() { return L"TabBand"; }

  public: virtual const char16* GetClass()  const override {
    return GetClass_();
  }

  private: typedef DoubleLinkedList_<Element> Elements;

  private: gfx::Graphics m_gfx;
  private: int m_cItems;
  private: BOOL m_compositionEnabled;
  private: int m_cxTab;
  private: int m_cxMinTab;
  private: Drag m_eDrag;
  private: bool m_fMouseTracking;
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

  // ctor
  private: TabBand(HWND hwnd) :
      m_cItems(0),
      m_compositionEnabled(false),
      m_cxTab(0),
      m_cxMinTab(k_cxMinTab),
      m_eDrag(Drag_None),
      m_fMouseTracking(false),
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
      m_xTab(0),
      Element(nullptr) {
    m_oElements.Append(&m_oListButton);
    COM_VERIFY(::DwmIsCompositionEnabled(&m_compositionEnabled));
  }

  // dotr
  private: ~TabBand() {
    if (auto const text_format = m_gfx.work<gfx::TextFormat>())
        delete text_format;

    if (m_hwndToolTips && (m_nStyle & TCS_TOOLTIPS) != 0) {
      ::DestroyWindow(m_hwndToolTips);
    }

    if (m_hTabListMenu) {
      ::DestroyMenu(m_hTabListMenu);
    }
  }

  // [C]
  private: bool changeFont(const gfx::Graphics& gfx) {
    LOGFONT lf;
    if (!::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
      return false;

    //lf.lfHeight = -13;

    if (auto const old_format = gfx.work<gfx::TextFormat*>())
        delete old_format;
    gfx.set_work(new gfx::TextFormat(lf));
    return true;
  }

  private: bool UpdateLayout() {
    if (m_cItems == 0) {
      m_cxTab = -1;
      m_xTab = -1;
      return false;
    }

    *m_oListButton.GetRect() = m_rc;

    m_oListButton.GetRect()->left = m_rc.left + k_cxMargin;

    auto x = m_oListButton.GetRect()->left;

    if (m_cItems >= 2) {
      m_oListButton.Show(true);
      x += k_cxListButton;
    } else {
      m_oListButton.Show(false);
    }

    m_oListButton.GetRect()->right = x;

    int cxTabs = m_rc.right - x - k_cxMargin;

    int cxTab = cxTabs / m_cItems;
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

    Item* pStartItem = nullptr;
    foreach (Elements::Enum, oEnum, &m_oElements) {
      Item* pItem = oEnum.Get()->DynamicCast<Item>();
      if (pItem) {
        pStartItem = pItem;
        break;
      }
    }

    ASSERT(pStartItem != nullptr);

    do {
      bool fShow = false;
      x = m_xTab;
      foreach (Elements::Enum, oEnum, &m_oElements) {
        Item* pItem = oEnum.Get()->DynamicCast<Item>();
        if (!pItem) {
          continue;
        }

        if (pItem == pStartItem) {
          fShow = true;
        }

        pItem->Show(fShow);

        if (!fShow) {
          continue;
        }

        RECT* prc = pItem->GetRect();
        prc->left = x;
        prc->right = x + cxTab;
        prc->top = m_rc.top;
        prc->bottom = m_rc.bottom;
        pItem->ComputeLayout();

        x += cxTab;

        fShow = x + cxTab < m_rc.right;
      }

      if (!m_pSelected) {
        // No tab is selected. So, we display from the first tab.
        break;
      }

      if (m_pSelected->IsShow()) {
        // Selected tab is visible. So, we don't need to scroll tabs.
        break;
      }

      Element* pNext = pStartItem;

      for (;;) {
        pNext = pNext->GetNext();
        if (!pNext) {
          pStartItem = nullptr;
          break;
        }

        pStartItem = pNext->DynamicCast<Item>();
        if (pStartItem) {
          break;
        }
      }
    } while (pStartItem);

    if (m_hwndToolTips) {
      TOOLINFO ti;
      ti.cbSize = sizeof(ti);
      ti.hwnd = m_hwnd;
      ti.uFlags = 0;

      foreach (Elements::Enum, oEnum, &m_oElements) {
        auto const pElement = oEnum.Get();
        if (pElement->Is<Item>()) {
          ti.uId = static_cast<uint>(pElement->StaticCast<Item>()->m_iItem);

        } else if (pElement->Is<ListButton>()) {
          ti.uId = k_TabListId;
        } else {
          continue;
        }

        if (pElement->IsShow()) {
          ti.rect = *pElement->GetRect();
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

  // [D]
  private: virtual void Draw(const gfx::Graphics& gfx) const override {
    do {
      gfx.BeginDraw();
      gfx->SetTransform(D2D1::IdentityMatrix());
      gfx->Clear(gfx::sysColor(COLOR_3DFACE,
                               m_compositionEnabled ? 0.0f : 1.0f));

      foreach (Elements::Enum, oEnum, &m_oElements) {
        auto const element = oEnum.Get();
        if (element->IsShow())
          element->Draw(gfx);
      }

      if (m_pInsertBefore)
          drawInsertMarker(m_gfx, m_pInsertBefore->GetRect());
    } while (!const_cast<gfx::Graphics&>(gfx).EndDraw());
  }

  private: static void drawInsertMarker(const gfx::Graphics& gfx, RECT* prc) {
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

  // [F]
  // findItem
  private: Item* findItem(int iItem) const {
    if (iItem < 0 || iItem >= m_cItems) {
      return nullptr;
    }

    foreach (Elements::Enum, oEnum, &m_oElements) {
      auto const pItem = oEnum.Get()->DynamicCast<Item>();
      if (!pItem) {
        continue;
      }

      if (pItem->m_iItem == iItem) {
        return pItem;
      }
    }

    return nullptr;
  }

  // findItemFromPoint
  private: Item* findItemFromPoint(POINT pt) const {
    foreach (Elements::Enum, oEnum, &m_oElements) {
      auto const pItem = oEnum.Get()->DynamicCast<Item>();
      if (!pItem) {
        continue;
      }

      if (pt.x < pItem->GetRect()->left) {
        break;
      }

      if (pItem->HitTest(pt) == pItem) {
        return pItem;
      }
    }
    return nullptr;
  }

  // [H]

  // handleTabListMenu
  private: void handleTabListMenu(POINT) {
    POINT ptMouse;
    ptMouse.x = m_oListButton.GetRect()->left;
    ptMouse.y = m_oListButton.GetRect()->bottom;

    ::ClientToScreen(m_hwnd, &ptMouse);

    if (!m_hTabListMenu) {
      m_hTabListMenu = ::CreatePopupMenu();
    }

    // Make Tab List Menu empty
    while (::GetMenuItemCount(m_hTabListMenu) > 0) {
      ::DeleteMenu(m_hTabListMenu, 0, MF_BYPOSITION);
    }

    // Add Tab name to menu.
    Item* pPrevItem = nullptr;
    foreach (Elements::Enum, oEnum, &m_oElements) {
      auto const pItem = oEnum.Get()->DynamicCast<Item>();
      if (!pItem) {
        continue;
      }

      auto const rgfFlag =
        pItem->IsSelected()
            ? MF_STRING | MF_CHECKED
            : MF_STRING;

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
          static_cast<uint>(rgfFlag),
          static_cast<uint>(pItem->m_iItem),
          pItem->label_.c_str());
    }

    ::TrackPopupMenuEx(
        m_hTabListMenu,
        TPM_LEFTALIGN | TPM_TOPALIGN,
        ptMouse.x, ptMouse.y,
        m_hwnd,
        nullptr);
  }

  // hitTest
  private: Element* hitTest(POINT pt) const {
    if (auto const pHit = m_oListButton.HitTest(pt)) {
      return pHit;
    }

    foreach (Elements::Enum, oEnum, &m_oElements) {
      auto const pItem = oEnum.Get()->DynamicCast<Item>();
      if (!pItem) {
        continue;
      }

      if (pt.x < pItem->GetRect()->left) {
        break;
      }

      if (auto const pElement = pItem->HitTest(pt)) {
        return pElement;
      }
    }

    return nullptr;
  }

  // [I]
  // Init
  public: static BOOL Init(HINSTANCE hInstance) {
    WNDCLASSEXW oWC;
    oWC.cbSize = sizeof(oWC);
    oWC.style = CS_DBLCLKS | CS_BYTEALIGNCLIENT;
    oWC.lpfnWndProc = windowProc;
    oWC.cbClsExtra = 0;
    oWC.cbWndExtra = 0;
    oWC.hInstance = hInstance;
    oWC.hIcon = nullptr;
    oWC.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    oWC.hbrBackground = nullptr;
    oWC.lpszMenuName = nullptr;
    oWC.lpszClassName = WC_TABBANDCLASS;
    oWC.hIconSm = nullptr;

    g_hInstance = hInstance;

    return ::RegisterClassExW(&oWC);
  }

  // [O]
  private: LRESULT onCreate(CREATESTRUCT* pcs) {
    m_gfx.Init(m_hwnd);
    changeFont(m_gfx);

    if (pcs->style & TCS_TOOLTIPS) {
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

    return TRUE;
  }

  // onDeleteItem
  private: bool OnDeleteItem(int iDeleteItem) {
    auto const pItem = findItem(iDeleteItem);
    if (!pItem) {
      return FALSE;
    }

    bool fSelChanged = m_pSelected == pItem;

    if (fSelChanged) {
      if (pItem->GetPrev()) {
        m_pSelected = pItem->GetPrev()->DynamicCast<Item>();
      }

      if (m_pSelected == nullptr && pItem->GetNext()) {
        m_pSelected = pItem->GetNext()->DynamicCast<Item>();
      }
    }

    if (m_pHover == pItem) {
      m_pHover = nullptr;
    }

    m_oElements.Delete(pItem);
    m_cItems -= 1;

    // Renumber tab index
    {
      int iItem = 0;
      foreach (Elements::Enum, oEnum, &m_oElements) {
        Item* pItem = oEnum.Get()->DynamicCast<Item>();
        if (!pItem) {
          continue;
        }

        pItem->m_iItem = iItem;
        iItem += 1;
      }
    }

    if (m_hwndToolTips) {
      TOOLINFO ti;
      ti.cbSize = sizeof(ti);
      ti.hwnd = m_hwnd;
      ti.uId = static_cast<uint>(m_cItems);
      ::SendMessage(
          m_hwndToolTips,
          TTM_DELTOOL,
          0,
          reinterpret_cast<LPARAM>(&ti));
    }

    Redraw();

    if (fSelChanged) {
      if (m_pSelected) {
        m_pSelected->SetState(Element::State_Selected);
      }

      sendNotify(TCN_SELCHANGE);
    }

    return TRUE;
  }

  // onInsertItem
  private: int onInsertItem(int iItem, const TCITEM* pTcItem) {
    auto const pNewItem = new Item(this, iItem, pTcItem);
    if (!pNewItem) {
      return -1;
    }

    pNewItem->m_iItem = iItem;

    if (m_iFocus >= iItem) {
      m_iFocus += 1;
    }

    Item* pRefItem = nullptr;
    foreach (Elements::Enum, oEnum, &m_oElements) {
      auto const pItem = oEnum.Get()->DynamicCast<Item>();
      if (!pItem) {
        continue;
      }

      if (pItem->m_iItem < iItem) {
        continue;
      }

      if (pItem->m_iItem == iItem) {
        ASSERT(!pRefItem);
        pRefItem = pItem;
      }

      pItem->m_iItem += 1;
    }

    m_cItems += 1;

    m_oElements.InsertBefore(pNewItem, pRefItem);

    if (m_hwndToolTips) {
      TOOLINFO ti;
      ti.cbSize = sizeof(ti);
      ti.hwnd = m_hwnd;
      ti.lpszText = LPSTR_TEXTCALLBACK;
      ti.uFlags = 0;
      ti.uId = static_cast<uint>(m_cItems - 1);
      ::SendMessage(
          m_hwndToolTips,
          TTM_ADDTOOL,
          0,
          reinterpret_cast<LPARAM>(&ti));
    }

    Redraw();

    return iItem;
  }

  // onLButtonDown
  private: void onLButtonDown(POINT pt) {
    auto const pElement = hitTest(pt);
    if (!pElement) {
      return;
    }

    auto const pItem = pElement->DynamicCast<Item>();
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

  // onLButtonUp
  private: void onLButtonUp(POINT pt) {
    if (!m_pDragItem) {
      auto const pElement = hitTest(pt);
      if (!pElement) {
        return;
      }

      if (pElement->Is<CloseBox>()) {
        if (auto const item = pElement->GetParent()->DynamicCast<Item>())
          sendNotify(TABBAND_NOTIFY_CLICK_CLOSE_BUTTON, item->m_iItem);
        return;
      }

      if (pElement->Is<ListButton>()) {
        handleTabListMenu(pt);
        return;
      }
    } else {
      Item* pDragItem = m_pDragItem;
      Item* pInsertBefore = m_pInsertBefore;

      stopDrag();

      if (!pInsertBefore) {
        handleTabDragAndDrop(m_hwnd, pt, kDrop);

      } else {
        if (pDragItem != pInsertBefore) {
          m_oElements.Delete(pDragItem);
          m_oElements.InsertBefore(pDragItem, pInsertBefore);
          int iItem = 0;
          foreach (Elements::Enum, oEnum, &m_oElements) {
            Item* pItem = oEnum.Get()->DynamicCast<Item>();
            if (!pItem) continue;
            pItem->m_iItem = iItem;
            iItem += 1;
          }

          UpdateLayout();
        }

        // Hide insertion position mark
        ::InvalidateRect(m_hwnd, nullptr, false);
      }
    }
  }

  // onMessage
  private: LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    #if DEBUG_MESSAGE
      DEBUG_PRINTF("%p msg=0x%x\n", this, uMsg);
    #endif // DEBUG_MESSAGE

    switch (uMsg) {
      case WM_COMMAND: {
        int const iItem = LOWORD(wParam);
        if (iItem >= 0) {
          SelectItem(iItem);
        }
        return 0;
      }

      case WM_CREATE:
        return onCreate(reinterpret_cast<CREATESTRUCT*>(lParam));

      case WM_DWMCOMPOSITIONCHANGED:
        if (FAILED(DwmIsCompositionEnabled(&m_compositionEnabled)))
            m_compositionEnabled = false;
        break;

      case WM_LBUTTONDOWN: {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        onLButtonDown(pt);
        return 0;
      }

      case WM_LBUTTONUP: {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        onLButtonUp(pt);
        return 0;
      }

      case WM_MOUSELEAVE: {
        #if DEBUG_HOVER
          DEBUG_PRINTF("WM_MOUSELEAVE %p hover=%ls.%p\n",
            this,
            m_pHover ? m_pHover->GetClass() : L"null",
            m_pHover);
        #endif // DEBUG_HOVER

        m_fMouseTracking = false;
        UpdateHover(nullptr);
        return 0;
      }

      case WM_MOUSEMOVE: {
        POINT const pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        OnMouseMove(pt);
        return 0;
      }

      case WM_NCDESTROY:
        delete this;
        break;

      case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (::ScreenToClient(m_hwnd, &pt)) {
          if (!hitTest(pt)) {
            return ::SendMessage(::GetParent(m_hwnd), uMsg, wParam, lParam);
          }
        }
        break;
      }

      case WM_NCLBUTTONDOWN:
      case WM_NCLBUTTONUP:
      case WM_NCMOUSEMOVE:
      case WM_NCRBUTTONDOWN:
      case WM_NCRBUTTONUP:
        return ::SendMessage(::GetParent(m_hwnd), uMsg, wParam, lParam);

      case WM_NOTIFY: {
        #if DEBUG_TOOLTIP
        {
          auto const p = reinterpret_cast<NMHDR*>(lParam);
          DEBUG_PRINTF("WM_NOTIFY %p ctrl=%d code=%d\n",
            this, wParam, p->code);
        }
        #endif

        return ::SendMessage(
            ::GetParent(m_hwnd),
            WM_NOTIFY,
            wParam,
            lParam);
      }

      case WM_PAINT:
        Draw(m_gfx);
        ValidateRect(m_hwnd, nullptr);
        return 0;

      case WM_SIZE: {
        // Handle WM_SIZE at window creation. We won't receive WM_SIZE
        // since we handle WM_WINDOWPOSCHANGED.
        auto const hwndParent = ::GetParent(m_hwnd);
        if (!hwndParent) {
          return 0;
        }

        ::GetClientRect(hwndParent, &m_rc);
        auto const iFontHeight = 16;  // must be >= 16 (Small Icon Height)
        //m_rc.bottom = m_rc.top + 6 + iFontHeight + 10;
        m_rc.bottom = m_rc.top + 2 + 7 + iFontHeight + 5 + 2;

        ::SetWindowPos(
            m_hwnd,
            nullptr,
            m_rc.left,
            m_rc.top,
            m_rc.right - m_rc.left,
            m_rc.bottom - m_rc.top,
            SWP_NOZORDER);
        return 0;
      }

      case WM_SETTINGCHANGE:
        switch (wParam) {
          case SPI_SETICONTITLELOGFONT:
          case SPI_SETNONCLIENTMETRICS: {
            changeFont(m_gfx);
            break;
          }
        }
        break;

      case WM_USER:
        return static_cast<LRESULT>(::GetSysColor(COLOR_3DFACE));

      case WM_WINDOWPOSCHANGED: {
        auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
        if (wp->flags & SWP_NOSIZE) {
          return 0;
        }

        m_rc.left = wp->x;
        m_rc.top = wp->y;
        m_rc.right = wp->x + wp->cx;
        m_rc.bottom = wp->y + wp->cy;

        m_gfx.Resize(m_rc);
        Redraw();
        return 0;
      }

      ////////////////////////////////////////////////////////////
      //
      // Tab Control Messages
      //
      case TCM_DELETEITEM:
        return OnDeleteItem(static_cast<int>(wParam));

      case TCM_GETCURFOCUS:
        return m_iFocus;

      case TCM_GETCURSEL:
        return m_pSelected ? m_pSelected->m_iItem : -1;

      case TCM_GETIMAGELIST:
        return reinterpret_cast<LRESULT>(m_hImageList);

      case TCM_GETITEM: {
        auto const pItem = findItem(static_cast<int>(wParam));
        if (!pItem) {
          return FALSE;
        }

        auto const pTcItem = reinterpret_cast<TCITEM*>(lParam);

        if (pTcItem->mask & TCIF_IMAGE) {
          pTcItem->iImage = pItem->m_iImage;
        }

        if (pTcItem->mask & TCIF_PARAM) {
          pTcItem->lParam = pItem->m_lParam;
        }

        if (pTcItem->mask & TCIF_STATE) {
          pTcItem->dwState = pItem->m_rgfState & pTcItem->dwStateMask;
        }

        if (pTcItem->mask & TCIF_TEXT) {
          auto const cwch = std::min(
            pItem->label_.length(),
            static_cast<size_t>(pTcItem->cchTextMax - 1));

          ::CopyMemory(
              pTcItem->pszText,
              pItem->label_.data(),
              sizeof(base::char16) * cwch);

          pTcItem->pszText[cwch] = 0;
        }
        return TRUE;
      }

      case TCM_GETITEMCOUNT:
        return m_cItems;

      case TCM_GETTOOLTIPS:
        return reinterpret_cast<LRESULT>(m_hwndToolTips);

      case TCM_INSERTITEM:
        return onInsertItem(
            static_cast<int>(wParam),
            reinterpret_cast<TCITEM*>(lParam));

      case TCM_SETCURFOCUS:
        m_iFocus = static_cast<int>(wParam);
        return 0;

      case TCM_SETCURSEL:
        return SelectItem(static_cast<int>(wParam));

      case TCM_SETIMAGELIST: {
        auto const hOldImageList = m_hImageList;
        SetImageList(reinterpret_cast<HIMAGELIST>(lParam));
        Redraw();
        return reinterpret_cast<LRESULT>(hOldImageList);
      }

      case TCM_SETITEM:
        if (auto const item = findItem(static_cast<int>(wParam))) {
          if (item->SetItem(reinterpret_cast<TCITEM*>(lParam)))
            item->Invalidate(m_hwnd);
        }
        return FALSE;

      case TCM_SETMINTABWIDTH: {
        auto const iPrev = m_cxMinTab;
        m_cxMinTab = std::max(static_cast<int>(lParam),
                              static_cast<int>(k_cxMinTab));
        return iPrev;
      }

      case TCM_SETTOOLTIPS:
        m_hwndToolTips = reinterpret_cast<HWND>(wParam);
        return 0;
    }

    return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
  }

  private: void OnMouseMove(POINT pt) {
    auto const pHover = hitTest(pt);

    if (!m_pDragItem) {
      // Hover
      if (!m_fMouseTracking) {
        TRACKMOUSEEVENT oTrack;
        oTrack.cbSize = sizeof(oTrack);
        oTrack.dwFlags = TME_LEAVE;
        oTrack.hwndTrack = m_hwnd;

        if (!::TrackMouseEvent(&oTrack)) {
          return;
        }

        m_fMouseTracking = true;
      }
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
      auto const pInsertBefore = pHover == nullptr ?
        nullptr :
        pHover->DynamicCast<Item>();

      auto hCursor = s_hDragTabCursor;

      if (!pInsertBefore) {
        #if 0
        if (handleTabDragAndDrop(m_hwnd, pt, kHover) == nullptr) {
          hCursor = ::LoadCursor(nullptr, MAKEINTRESOURCE(IDC_NO));
        }
        #endif
      }

      ::SetCursor(hCursor);

      if (pInsertBefore != m_pInsertBefore) {
        ::InvalidateRect(m_hwnd, nullptr, false);
      }

      m_pInsertBefore = pInsertBefore;
    }
  }

  // [R]
  private: void UpdateHover(Element* pHover) {
      if (m_pHover == pHover)
        return;

    if (m_pHover) {
      if (!pHover || !pHover->Is<CloseBox>() ||
          pHover->GetParent() != m_pHover) {
        if (m_pHover->Is<CloseBox>()) {
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

  // [S]
  // selectItem
  private: int SelectItem(int const iItem) {
    return SelectItem(findItem(iItem));
  }

  private: int SelectItem(Item* const pItem) {
    if (m_pSelected != pItem) {
      if (m_pSelected) {
        m_pSelected->SetState(Element::State_Normal);
        m_pSelected->Invalidate(m_hwnd);
      }

      m_pSelected = pItem;

      if (pItem) {
        pItem->SetState(Element::State_Selected);
        if (!pItem->IsShow()) {
          Redraw();
        }
        pItem->Invalidate(m_hwnd);
      }

      sendNotify(TCN_SELCHANGE);
    }

    return m_pSelected ? m_pSelected->m_iItem : -1;
  }

  // sendNotify
  private: LRESULT sendNotify(uint const nCode, int tab_index = 0) {
    auto const hwndParent = ::GetParent(m_hwnd);

    if (!hwndParent) {
      return TRUE;
    }

    TabBandNotifyData oNotify;
    oNotify.code = nCode;
    oNotify.hwndFrom = m_hwnd;
    oNotify.idFrom = static_cast<uint>(::GetDlgCtrlID(m_hwnd));
    oNotify.tab_index_ = tab_index;

    return ::SendMessage(
      hwndParent,
      WM_NOTIFY,
      oNotify.idFrom,
      reinterpret_cast<LPARAM>(&oNotify));
  }

  private: void stopDrag() {
    m_eDrag = Drag_None;
    m_pDragItem = nullptr;
    m_pInsertBefore = nullptr;

    ::ReleaseCapture();
    ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
  }

  // [U]
  private: void Redraw() {
    UpdateLayout();
    ::InvalidateRect(m_hwnd, nullptr, false);
  }

  // [W]
  private: static LRESULT CALLBACK windowProc(
      HWND const hwnd,
      UINT const uMsg,
      WPARAM const wParam,
      LPARAM const lParam) {
    auto tabBand = reinterpret_cast<TabBand*>(
      ::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (!tabBand) {
      tabBand = new TabBand(hwnd);

      ::SetWindowLongPtr(
        hwnd,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(tabBand));
    }

    return tabBand->OnMessage(uMsg, wParam, lParam);
  }

  DISALLOW_COPY_AND_ASSIGN(TabBand);
};

}

void TabBand__Init(HINSTANCE const hInstance) {
  TabBand::Init(hInstance);
}
