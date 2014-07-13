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
#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/castable.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/events/event.h"
#include "evita/ui/tooltip.h"
#include "evita/views/frame_list.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/vi_Frame.h"

namespace {

void fillRect(gfx::Canvas* canvas, int x, int y, int cx, int cy) {
  RECT rc;
  rc.left = x;
  rc.right = x + cx;
  rc.top = y;
  rc.bottom = y + cy;
  gfx::Brush brush(canvas, gfx::blackColor());
  canvas->FillRectangle(brush, rc);
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

  protected: RECT bounds_;
  protected: HIMAGELIST image_list_;
  protected: bool is_hover_;
  protected: bool is_shown_;
  protected: Element* parent_;
  protected: State state_;

  // ctor
  protected: Element(Element* pParent);
  public: virtual ~Element() = default;

  protected: gfx::ColorF bgcolor() const;
  public: RECT* bounds() { return &bounds_; }
  public: Element* parent() const { return parent_; }
  public: State state() const { return state_; }

  // [D]
  public: virtual void Draw(gfx::Canvas* canvas) const = 0;

  // [G]
  public: HIMAGELIST GetImageList() const;

  // [H]
  public: virtual Element* HitTest(POINT point) const;

  // [I]
  public: void Invalidate(HWND hwnd);
  public: bool IsDescendantOf(const Element* other) const;
  public: bool IsHover() const { return is_hover_; }
  public: bool IsSelected() const { return State_Selected == state_; }
  public: bool IsShow() const { return is_shown_; }

  // [S]
  public: bool SetHover(bool f);
  public: void SetImageList(HIMAGELIST hImageList);
  public: Element* SetParent(Element* p);
  public: State SetState(State e);
  public: bool Show(bool f);

  // [U]
  protected: virtual void Update();
};

Element::Element(Element* pParent)
    : image_list_(nullptr),
      is_hover_(false),
      is_shown_(true),
      parent_(pParent),
      state_(State_Normal) {
  bounds_.left = bounds_.top = bounds_.right = bounds_.top = 0;
}

gfx::ColorF Element::bgcolor() const {
  if (IsSelected())
      return gfx::whiteColor();
  if (IsHover())
      return gfx::sysColor(COLOR_3DHILIGHT, 0.8);
  return gfx::sysColor(COLOR_3DFACE, 0.5);
}

HIMAGELIST Element::GetImageList() const {
  for (const Element* runner = this; runner; runner = runner->parent_) {
    if (auto const image_list = runner->image_list_)
      return image_list;
  }
  return nullptr;
}

Element* Element::HitTest(POINT pt) const {
  return IsShow() && ::PtInRect(&bounds_, pt) ? const_cast<Element*>(this) :
                                                nullptr;
}

void Element::Invalidate(HWND hwnd) {
  // TODO(yosi) We should use GFX version of invalidate rectangle.
  ::InvalidateRect(hwnd, &bounds_, false);
}

bool Element::IsDescendantOf(const Element* other) const {
  for (auto runner = parent_; runner; runner = runner->parent_) {
    if (runner == other)
      return true;
  }
  return false;
}

bool Element::SetHover(bool f) {
  return is_hover_ = f;
}

void Element::SetImageList(HIMAGELIST hImageList) {
  image_list_ = hImageList;
}

Element* Element::SetParent(Element* p) {
  return parent_ = p;
}

Element::State Element::SetState(State e) {
  state_ = e;
  Update();
  return state_;
}

bool Element::Show(bool f) {
  return is_shown_ = f;
}

void Element::Update() {
}

//////////////////////////////////////////////////////////////////////
//
// TabStripImpl Design Parameters
//
enum TabStripImplDesignParams {
  k_cxMargin = 0,
  k_cyMargin = 4,
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
  private: void DrawXMark(gfx::Canvas* canvas, gfx::ColorF color) const;
  private: gfx::ColorF markColor() const;

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(CloseBox);
};

CloseBox::CloseBox(Element* pParent) : Element(pParent) {
}

void CloseBox::DrawXMark(gfx::Canvas* canvas, gfx::ColorF color) const {
  gfx::Brush brush(canvas, color);

  RECT rc = bounds_;
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
    canvas->FillRectangle( \
      brush, \
      bounds_.left + x, bounds_.top + y, \
      bounds_.left + x + cx, bounds_.top + y + cy);

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

void CloseBox::Draw(gfx::Canvas* canvas) const {
  DrawXMark(canvas, markColor());
}

//////////////////////////////////////////////////////////////////////
//
// Tab
//  Represents tab tab.
//
class Tab final : public Element, public ui::Tooltip::ToolDelegate {
  DECLARE_CASTABLE_CLASS(Tab, Element);

  private: enum Design {
    k_cxCloseBoxMargin = 3,
    k_cyCloseBoxMargin = 9,
    k_cyDescent = 4,
  };

  private: CloseBox close_box_;
  public: int image_index_;
  private: RECT label_bounds_;
  public: base::string16 label_text_;
  public: LPARAM lparam_;
  public: uint32_t state_;
  private: int tab_index_;
  private: views::TabStripDelegate* tab_strip_delegate_;

  public: Tab(views::TabStripDelegate* tab_strip_delegate,
              Element* pParent, const TCITEM* pTcItem);
  public: virtual ~Tab() = default;

  public: int tab_index() const { return tab_index_; }
  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  public: void ComputeLayout();
  private: void DrawContent(gfx::Canvas* canvas) const;
  private: void DrawIcon(gfx::Canvas* canvas) const;
  public: bool HasCloseBox() const;
  public: bool SetTab(const TCITEM* pTcItem);

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;
  public: virtual Element* HitTest(POINT point) const override;
  private: void Update() override;

  // ui::Tooltip::ToolDelegate
  private: base::string16 GetTooltipText() override;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

Tab::Tab(views::TabStripDelegate* tab_strip_delegate, Element* pParent,
         const TCITEM* pTcItem)
    : Element(pParent),
      close_box_(this),
      image_index_(-1),
      tab_index_(0),
      state_(0),
      tab_strip_delegate_(tab_strip_delegate) {
  SetTab(pTcItem);
}

void Tab::ComputeLayout() {
  label_bounds_ = bounds_;

  auto const prc = close_box_.bounds();
  *prc = bounds_;

  prc->right  -= k_cxCloseBoxMargin;
  prc->left = prc->right - CloseBox::Width;
  prc->top  += k_cyCloseBoxMargin;
  prc->bottom = prc->top + CloseBox::Height;

  label_bounds_.right = prc->left;

  label_bounds_.left += k_cxBorder + k_cxEdge;
  label_bounds_.right -= k_cxBorder + k_cxEdge;
  label_bounds_.top  += 6 + 4;
  label_bounds_.bottom = label_bounds_.top + 12;

  if (image_index_ >= 0) {
    label_bounds_.left += 16 + 4;
  }
}

void Tab::DrawContent(gfx::Canvas* canvas) const {
  DrawIcon(canvas);

  // Label Text
  {
    RECT rc = bounds_;
    rc.left += 4 + 16 + 4;
    rc.right -= 4;
    rc.top += 8;
    rc.bottom = rc.bottom - 2;

    gfx::Brush brush(canvas, gfx::sysColor(COLOR_BTNTEXT));
    canvas->DrawText(*canvas->work<gfx::TextFormat>(), brush, rc,
                    label_text_.data(), label_text_.length());
  }
}

void Tab::DrawIcon(gfx::Canvas* canvas) const {
  if (image_index_ < 0)
    return;
  auto const hImageList = GetImageList();
  if (!hImageList)
    return;
  // Note: ILD_TRANSPARENT doesn't effect.
  // Note: ILD_DPISCALE makes background black.
  auto const hIcon = ::ImageList_GetIcon(
      hImageList,
      image_index_,
      0);
  if (!hIcon)
    return;
  gfx::Bitmap bitmap(*canvas, hIcon);
  auto const icon_size = canvas->AlignToPixel(gfx::SizeF(16, 16));
  auto const icon_offset = canvas->AlignToPixel(gfx::SizeF(-20, 8));
  auto const icon_left_top = gfx::PointF(label_bounds_.left, bounds_.top) +
                             icon_offset;
  (*canvas)->DrawBitmap(bitmap, gfx::RectF(icon_left_top, icon_size));
  ::DestroyIcon(hIcon);
}

bool Tab::HasCloseBox() const {
  return IsSelected() || IsHover();
}

bool Tab::SetTab(const TCITEM* pTcItem) {
  auto changed = false;
  if (pTcItem->mask & TCIF_IMAGE) {
    if (image_index_ != pTcItem->iImage)
      changed = true;
    image_index_ = pTcItem->iImage;
  }

  if (pTcItem->mask & TCIF_PARAM) {
    lparam_ = pTcItem->lParam;
  }

  if (pTcItem->mask & TCIF_STATE) {
    auto const old_state = state_;
    state_ &= ~pTcItem->dwStateMask;
    state_ |= pTcItem->dwState & pTcItem->dwStateMask;
    if (state_ != old_state)
      changed = true;
  }

  if (pTcItem->mask & TCIF_TEXT) {
    auto const new_label_text = base::string16(pTcItem->pszText);
    if (label_text_ != new_label_text) {
      label_text_ = new_label_text;
      changed = true;
    }
  }
  return changed;
}

// Element
void Tab::Draw(gfx::Canvas* canvas) const {
  #if DEBUG_HOVER
    DEBUG_PRINTF("%p sel=%d %ls\n",
      this,
      IsSelected(),
      label.c_str());
  #endif

  {
    RECT rc = bounds_;
    gfx::Brush fillBrush(canvas, bgcolor());
    canvas->FillRectangle(fillBrush, rc);
    gfx::Brush strokeBrush(canvas, gfx::blackColor());
    canvas->DrawRectangle(strokeBrush, rc, 0.2);
  }

  DrawContent(canvas);
  if (HasCloseBox())
    close_box_.Draw(canvas);
  if (!state_)
    return;
  auto const marker_height = 4;
  auto const marker_width = 4;
  DCHECK_GT(bounds_.right - bounds_.left, marker_width);
  canvas->FillRectangle(
      gfx::Brush(canvas, gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255)),
      gfx::Rect(gfx::Point(bounds_.right - marker_width, bounds_.top),
                gfx::Size(marker_width, marker_height)));
}

Element* Tab::HitTest(POINT pt) const {
  if (HasCloseBox()) {
    if (auto const hit = close_box_.HitTest(pt))
      return hit;
  }
  return Element::HitTest(pt);
}

void Tab::Update() {
  ComputeLayout();
}

// ui::Tooltip::ToolDelegate
base::string16 Tab::GetTooltipText() {
  return tab_strip_delegate_->GetTooltipTextForTab(tab_index_);
}

//////////////////////////////////////////////////////////////////////
//
// ListButton
//
class ListButton final : public Element {
  DECLARE_CASTABLE_CLASS(ListButton, Element);

  public: ListButton(Element* pParent);
  public: virtual ~ListButton() = default;

  private: void drawDownArrow(gfx::Canvas* canvas) const;

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(ListButton);
};

ListButton::ListButton(Element* pParent) : Element(pParent) {
}

void ListButton::drawDownArrow(gfx::Canvas* canvas) const {
  auto const x = (bounds_.right - bounds_.left - 4) / 2 + bounds_.left;
  auto const y = (bounds_.bottom - bounds_.top) / 2 + bounds_.top;
  gfx::Brush arrowBrush(canvas, gfx::blackColor());
  canvas->FillRectangle(arrowBrush, x + 0, y + 0, 5, 1);
  canvas->FillRectangle(arrowBrush, x + 1, y + 1, 3, 1);
  canvas->FillRectangle(arrowBrush, x + 2, y + 2, 1, 1);
}

// Element
void ListButton::Draw(gfx::Canvas* canvas) const {
  ASSERT(IsShow());
  if (bounds_.left == bounds_.right)
      return;

  gfx::Brush fillBrush(canvas, bgcolor());
  canvas->FillRectangle(fillBrush, bounds_);
  gfx::Brush strokeBrush(canvas, gfx::blackColor());
  canvas->DrawRectangle(strokeBrush, bounds_, 0.2);

  // Draw triangle
  {
      auto const x = (bounds_.right - bounds_.left - 4) / 2 + bounds_.left;
      auto const y = (bounds_.bottom - bounds_.top) / 2 + bounds_.top;
      fillRect(canvas, x + 0, y + 0, 5, 1);
      fillRect(canvas, x + 1, y + 1, 3, 1);
      fillRect(canvas, x + 2, y + 2, 1, 1);
  }
}

// Cursor for Tab Drag
HCURSOR s_hDragTabCursor;

// Load cursor for Tab Drag and Drop.
void LoadDragTabCursor() {
  if (s_hDragTabCursor)
    return;

  s_hDragTabCursor = ::LoadCursor(nullptr, IDC_ARROW);

  auto const hDll = ::LoadLibraryEx(L"ieframe.dll", nullptr,
                                    LOAD_LIBRARY_AS_DATAFILE);
  if (!hDll)
    return;

  if (auto const hCursor = ::LoadCursor(hDll, MAKEINTRESOURCE(643))) {
    if (auto const hCursor2 = CopyCursor(hCursor))
      s_hDragTabCursor = hCursor2;
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
//  width_of_all_tabs_
//    Width of tab. width_of_all_tabs_ and tab_origin_ are update by
//    ComputeLayout. We use these variables to avoid redraw.
//  tab_origin_
//    Left point of the first tab. tab_origin_ can be negative if width of
//    Tab Band control is smaller than total number of tabs times
//    width_of_all_tabs_.
//
class TabStrip::TabStripImpl final : public Element {
  DECLARE_CASTABLE_CLASS(TabStripImpl, Element);

  friend class TabStrip;

  private: enum Constants {
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

  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: BOOL composition_enabled;
  private: TabStripDelegate* delegate_;
  private: Tab* dragging_tab_;
  private: Drag drag_state_;
  private: POINT drag_start_point_;
  private: Elements elements_;
  private: int focus_tab_index_;
  private: Element* hover_element_;
  private: HWND hwnd_;
  private: Tab* insertion_marker_;
  private: ListButton list_button_;
  private: long min_tab_width_;
  private: int num_tabs_;
  private: Tab* selected_tab_;
  private: HMENU tab_list_menu_;
  private: int tab_origin_;
  private: ui::Tooltip tooltip_;
  private: int width_of_all_tabs_;

  public: TabStripImpl(HWND hwnd, TabStripDelegate* delegate);
  public: virtual ~TabStripImpl();

  // [C]
  private: bool ChangeFont();

  // [D]
  private: void DidChangeTabSelection();
  public: void DidCreateNativeWindow();
  public: void DeleteTab(int iDeleteTab);
  private: void DropTab(Tab* tab, const POINT& point);

  // [F]
  private: Elements::iterator FindTab(int tab_index);

  // [G]
  private: Tab* GetTabFromIndex(int iTab) const;
  public: bool GetTab(int tab_index, TCITEM* pTcItem) const;

  // [H]
  private: void handleTabListMenu(POINT point);
  private: Element* HitTest(POINT point) const;

  // [I]
  private: void InsertTab(int iTab, const TCITEM* pTcItem);

  // [O]
  private: void OnLButtonDown(POINT pt);
  private: void OnLButtonUp(POINT pt);
  private: LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  private: void OnMouseMove(POINT pt);
  private: LRESULT OnNotify(NMHDR* nmhdr);

  // [R]
  public: void Redraw();
  private: void RenumberTabIndex();

  // [S]
  private: int SelectTab(int const iTab);
  private: int SelectTab(Tab* const pTab);
  private: void StopDrag();

  // [U]
  private: void UpdateHover(Element* pHover);
  private: bool UpdateLayout();

  // Element
  public: void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(TabStripImpl);
};

TabStrip::TabStripImpl::TabStripImpl(HWND hwnd, TabStripDelegate* delegate)
    : Element(nullptr),
      canvas_(new gfx::Canvas(gfx::Canvas::DwmSupport::SupportDwm)),
      composition_enabled(false),
      delegate_(delegate),
      dragging_tab_(nullptr),
      drag_state_(Drag_None),
      focus_tab_index_(-1),
      hover_element_(nullptr),
      hwnd_(hwnd),
      insertion_marker_(nullptr),
      list_button_(this),
      min_tab_width_(k_cxMinTab),
      num_tabs_(0),
      selected_tab_(nullptr),
      tab_list_menu_(nullptr),
      tab_origin_(0),
      width_of_all_tabs_(0) {
  elements_.push_back(&list_button_);
  COM_VERIFY(::DwmIsCompositionEnabled(&composition_enabled));
}

TabStrip::TabStripImpl::~TabStripImpl() {
  if (auto const text_format = canvas_->work<gfx::TextFormat>())
      delete text_format;

  if (tab_list_menu_)
    ::DestroyMenu(tab_list_menu_);
}

bool TabStrip::TabStripImpl::ChangeFont() {
  LOGFONT lf;
  if (!::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
    return false;

  if (auto const old_format = canvas_->work<gfx::TextFormat*>())
    delete old_format;
  canvas_->set_work(new gfx::TextFormat(lf));
  return true;
}

void TabStrip::TabStripImpl::DeleteTab(int iDeleteTab) {
  auto present = FindTab(iDeleteTab);
  if (present == elements_.end())
    return;

  auto const tab = (*present)->as<Tab>();
  auto const selection_changed = selected_tab_ == tab;
  if (selection_changed)
    selected_tab_ = GetTabFromIndex(iDeleteTab ? iDeleteTab - 1 : 1);

  if (hover_element_ && (hover_element_ == tab ||
                         hover_element_->IsDescendantOf(tab))) {
    hover_element_ = nullptr;
  }

  elements_.erase(present);
  RenumberTabIndex();
  tooltip_.DeleteTool(tab);
  delete tab;
  --num_tabs_;
  Redraw();
  if (selection_changed) {
    if (selected_tab_)
      selected_tab_->SetState(Element::State_Selected);
    DidChangeTabSelection();
  }
}

void TabStrip::TabStripImpl::DidChangeTabSelection() {
  delegate_->DidChangeTabSelection(selected_tab_ ? selected_tab_->tab_index() :
                                                   -1);
}

void TabStrip::TabStripImpl::DidCreateNativeWindow() {
  canvas_->Init(hwnd_);
  ChangeFont();
  tooltip_.Realize(hwnd_);
}

// Send TabDragMsg to window which can handle it.
void TabStrip::TabStripImpl::DropTab(Tab* tab, const POINT& ptClient) {
  auto ptScreen = ptClient;
  if (!::ClientToScreen(hwnd_, &ptScreen))
    return;

  for (auto hwnd = ::WindowFromPoint(ptScreen); hwnd;
       hwnd = ::GetParent(hwnd)) {
    if (auto const frame = FrameList::instance()->FindFrameByHwnd(hwnd)) {
      static_cast<TabStripDelegate*>(frame)->OnDropTab(tab->lparam_);
      return;
    }
  }

  delegate_->DidThrowTab(tab->lparam_);
}

TabStrip::TabStripImpl::Elements::iterator TabStrip::TabStripImpl::FindTab(
    int tab_index) {
  for (auto it = elements_.begin(); it != elements_.end(); ++it) {
    auto const tab = (*it)->as<Tab>();
    if (tab && tab->tab_index() == tab_index)
      return it;
  }
  return elements_.end();
}

bool TabStrip::TabStripImpl::GetTab(int tab_index, TCITEM* pTcItem) const {
  auto const tab = GetTabFromIndex(tab_index);
  if (!tab)
    return false;

  if (pTcItem->mask & TCIF_IMAGE)
    pTcItem->iImage = tab->image_index_;

  if (pTcItem->mask & TCIF_PARAM)
    pTcItem->lParam = tab->lparam_;

  if (pTcItem->mask & TCIF_STATE)
    pTcItem->dwState = tab->state_ & pTcItem->dwStateMask;

  if (pTcItem->mask & TCIF_TEXT) {
    auto const cwch = std::min(tab->label_text_.length(),
                               static_cast<size_t>(pTcItem->cchTextMax - 1));
    ::CopyMemory(pTcItem->pszText, tab->label_text_.data(),
                 sizeof(base::char16) * cwch);
    pTcItem->pszText[cwch] = 0;
  }
  return true;
}

Tab* TabStrip::TabStripImpl::GetTabFromIndex(int tab_index) const {
  for (auto element : elements_) {
    auto const tab = element->as<Tab>();
    if (tab && tab->tab_index() == tab_index)
      return tab;
  }
  return nullptr;
}

void TabStrip::TabStripImpl::handleTabListMenu(POINT) {
  POINT ptMouse;
  ptMouse.x = list_button_.bounds()->left;
  ptMouse.y = list_button_.bounds()->bottom;

  ::ClientToScreen(hwnd_, &ptMouse);

  if (!tab_list_menu_)
    tab_list_menu_ = ::CreatePopupMenu();

  // Make Tab List Menu empty
  while (::GetMenuItemCount(tab_list_menu_) > 0) {
    ::DeleteMenu(tab_list_menu_, 0, MF_BYPOSITION);
  }

  // Add Tab name to menu.
  Tab* last_tab = nullptr;
  for (auto element : elements_) {
    auto const tab = element->as<Tab>();
    if (!tab)
      continue;
    auto const rgfFlag = tab->IsSelected() ? MF_STRING | MF_CHECKED :
                                              MF_STRING;
    if (last_tab && last_tab->IsShow() != tab->IsShow())
      ::AppendMenu(tab_list_menu_, MF_SEPARATOR, 0, nullptr);
    last_tab = tab;
    ::AppendMenu(tab_list_menu_, static_cast<DWORD>(rgfFlag),
                 static_cast<DWORD>(tab->tab_index()),
                 tab->label_text_.c_str());
  }

  ::TrackPopupMenuEx(tab_list_menu_, TPM_LEFTALIGN | TPM_TOPALIGN, 
      ptMouse.x, ptMouse.y, hwnd_, nullptr);
}

Element* TabStrip::TabStripImpl::HitTest(POINT pt) const {
  if (auto const pHit = list_button_.HitTest(pt))
    return pHit;

  for (auto element : elements_) {
    auto const tab = element->as<Tab>();
    if (!tab)
      continue;

    if (pt.x < tab->bounds()->left)
      break;

    if (auto const hit = tab->HitTest(pt))
      return hit;
  }

  return nullptr;
}

void TabStrip::TabStripImpl::InsertTab(int tab_index, const TCITEM* pTcItem) {
  auto const new_tab = new Tab(delegate_, this, pTcItem);
  auto present = FindTab(tab_index);
  if (present == elements_.end()) {
    elements_.push_back(new_tab);
  } else {
    if (focus_tab_index_ >= tab_index)
      ++focus_tab_index_;
    elements_.insert(present, new_tab);
  }
  ++num_tabs_;
  RenumberTabIndex();
  tooltip_.AddTool(new_tab);
  Redraw();
}

void TabStrip::TabStripImpl::OnLButtonDown(POINT pt) {
  auto const element = HitTest(pt);
  if (!element)
    return;

  auto const tab = element->as<Tab>();
  if (!tab) {
    // Not a tab.
    return;
  }

  if (!tab->IsSelected()) {
    // Note: We should start tab dragging, otherwise if mouse pointer is in
    // close box, onButtonUp close the tab.
    SelectTab(tab);
  }

  #if DEBUG_DRAG
    DEBUG_PRINTF("%p drag=%p\n", this, dragging_tab_);
  #endif

  LoadDragTabCursor();

  dragging_tab_ = tab;
  drag_state_ = Drag_Start;
  drag_start_point_ = pt;

  ::SetCapture(hwnd_);
}

void TabStrip::TabStripImpl::OnLButtonUp(POINT pt) {
  if (!dragging_tab_) {
    auto const element = HitTest(pt);
    if (!element)
      return;

    if (element->is<CloseBox>()) {
      if (auto const tab = element->parent()->as<Tab>())
        delegate_->DidClickTabCloseButton(tab->tab_index());
      return;
    }

    if (element->is<ListButton>()) {
      handleTabListMenu(pt);
      return;
    }

    return;
  }

  auto const pDragTab = dragging_tab_;
  auto const pInsertBefore = insertion_marker_;
  StopDrag();

  if (!pInsertBefore) {
    DropTab(pDragTab, pt);
    return;
  }

  if (pDragTab != pInsertBefore) {
    elements_.erase(std::find(elements_.begin(), elements_.end(),
                                pDragTab));
    elements_.insert(std::find(elements_.begin(), elements_.end(),
                                 pInsertBefore),
                       pDragTab);
    RenumberTabIndex();
    UpdateLayout();
  }

  // Hide insertion position mark
  ::InvalidateRect(hwnd_, nullptr, false);
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
      if (FAILED(DwmIsCompositionEnabled(&composition_enabled)))
          composition_enabled = false;
      break;

    case WM_NCHITTEST: {
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      if (::ScreenToClient(hwnd_, &pt)) {
        if (!HitTest(pt))
          return ::SendMessage(::GetParent(hwnd_), uMsg, wParam, lParam);
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
      // management, e.g. moving top level window by grabbing empty area
      // of tabs.
      return ::SendMessage(::GetParent(hwnd_), uMsg, wParam, lParam);

    case WM_SETTINGCHANGE:
      switch (wParam) {
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETNONCLIENTMETRICS: {
          ChangeFont();
          break;
        }
      }
      break;
  }

  return 0;
}

void TabStrip::TabStripImpl::OnMouseMove(POINT pt) {
  auto const pHover = HitTest(pt);

  if (!dragging_tab_) {
    UpdateHover(pHover);
  } else {
    if (::GetCapture() != hwnd_) {
      // Someone takes capture. So, we stop dragging.
      StopDrag();
      return;
    }

    if (Drag_Start == drag_state_) {
      if (pt.x - drag_start_point_.x >= -5 &&
          pt.x - drag_start_point_.x <= 5) {
        return;
      }

      drag_state_ = Drag_Tab;
    }

    // Tab dragging
    auto const pInsertBefore = pHover ? pHover->as<Tab>() : nullptr;
    ::SetCursor(s_hDragTabCursor);
    if (pInsertBefore != insertion_marker_)
      ::InvalidateRect(hwnd_, nullptr, false);
    insertion_marker_ = pInsertBefore;
  }
}

LRESULT TabStrip::TabStripImpl::OnNotify(NMHDR* nmhdr) {
  tooltip_.OnNotify(nmhdr);
  return 0;
}

void TabStrip::TabStripImpl::Redraw() {
  UpdateLayout();
  ::InvalidateRect(hwnd_, nullptr, false);
}

void TabStrip::TabStripImpl::RenumberTabIndex() {
  auto tab_index = 0;
  for (auto element : elements_) {
    if (auto const tab = element->as<Tab>()) {
      tab->set_tab_index(tab_index);
      ++tab_index;
    }
  }
}

int TabStrip::TabStripImpl::SelectTab(int const iTab) {
  return SelectTab(GetTabFromIndex(iTab));
}

int TabStrip::TabStripImpl::SelectTab(Tab* const pTab) {
  if (selected_tab_ != pTab) {
    if (selected_tab_) {
      selected_tab_->SetState(Element::State_Normal);
      selected_tab_->Invalidate(hwnd_);
    }

    selected_tab_ = pTab;

    if (pTab) {
      pTab->SetState(Element::State_Selected);
      if (!pTab->IsShow())
        Redraw();
      pTab->Invalidate(hwnd_);
    }

    DidChangeTabSelection();
  }

  return selected_tab_ ? selected_tab_->tab_index() : -1;
}

void TabStrip::TabStripImpl::StopDrag() {
  drag_state_ = Drag_None;
  dragging_tab_ = nullptr;
  insertion_marker_ = nullptr;

  ::ReleaseCapture();
  ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

void TabStrip::TabStripImpl::UpdateHover(Element* pHover) {
    if (hover_element_ == pHover)
      return;

  if (hover_element_) {
    if (!pHover || !pHover->is<CloseBox>() ||
        pHover->parent() != hover_element_) {
      if (hover_element_->is<CloseBox>()) {
        hover_element_->SetHover(false);
        hover_element_ = hover_element_->parent();
      }
      hover_element_->SetHover(false);
      hover_element_->Invalidate(hwnd_);
    }
  }

  hover_element_ = pHover;
  if (hover_element_) {
    hover_element_->SetHover(true);
    hover_element_->Invalidate(hwnd_);
  }
}

bool TabStrip::TabStripImpl::UpdateLayout() {
  if (!num_tabs_) {
    width_of_all_tabs_ = -1;
    tab_origin_ = -1;
    return false;
  }

  const auto kMarginHeight = ::GetSystemMetrics(SM_CYSIZE) + k_cyMargin;
  *list_button_.bounds() = bounds_;
  list_button_.bounds()->left = bounds_.left + k_cxMargin;
  list_button_.bounds()->top = bounds_.top + kMarginHeight;

  auto x = list_button_.bounds()->left;

  if (num_tabs_ >= 2) {
    list_button_.Show(true);
    x += k_cxListButton;
  } else {
    list_button_.Show(false);
  }

  list_button_.bounds()->right = x;

  auto cxTabs = bounds_.right - x - k_cxMargin;
  auto cxTab = cxTabs / num_tabs_;
  cxTab = std::min(cxTab, min_tab_width_ * 2);

  if (cxTab >= min_tab_width_) {
    // We can show all tabs.
  } else {
    // How many tabs do we show in min width?
    int cVisibles = cxTabs / min_tab_width_;
    if (cVisibles == 0) {
      cVisibles = 1;
    }
    cxTab = cxTabs / cVisibles;
  }

  auto fChanged = width_of_all_tabs_ != cxTab || tab_origin_ != x;

  width_of_all_tabs_ = cxTab;
  tab_origin_ = x;

  for (auto view_start_tab_index = 0; view_start_tab_index < num_tabs_;
       ++view_start_tab_index) {
    auto fShow = false;
    x = tab_origin_;
    for (auto element : elements_) {
      auto const tab = element->as<Tab>();
      if (!tab)
        continue;

      if (tab->tab_index() == view_start_tab_index)
        fShow = true;

      tab->Show(fShow);

      if (!fShow)
        continue;

      RECT* prc = tab->bounds();
      prc->left = x;
      prc->right = x + cxTab;
      prc->top = bounds_.top + kMarginHeight;
      prc->bottom = bounds_.bottom;
      tab->ComputeLayout();

      x += cxTab;

      fShow = x + cxTab < bounds_.right;
    }

    if (!selected_tab_ || selected_tab_->IsShow())
      break;
  }

  for (auto element : elements_) {
    auto const tab = element->as<Tab>();
    if (!tab)
      continue;
    if (tab->IsShow())
      tooltip_.SetToolBounds(tab, *tab->bounds());
    else
      tooltip_.SetToolBounds(tab, Rect());
  }

  return fChanged;
}

// Element
void TabStrip::TabStripImpl::Draw(gfx::Canvas* canvas) const {
  struct Local {
    static void DrawInsertMarker(gfx::Canvas* canvas, RECT* prc) {
      auto rc = * prc;
      rc.top += 5;
      rc.bottom -= 7;

      for (int w = 1; w <= 7; w += 2) {
        fillRect(canvas, rc.left, rc.top, w, 1);
        fillRect(canvas, rc.left, rc.bottom, w, 1);

        rc.top  -= 1;
        rc.left   -= 1;
        rc.bottom += 1;
      }
    }
  };

  (*canvas)->Clear(gfx::sysColor(COLOR_3DFACE,
                                 composition_enabled ? 0.0f : 1.0f));

  for (auto element : elements_) {
    if (element->IsShow())
      element->Draw(canvas);
  }

  if (insertion_marker_)
    Local::DrawInsertMarker(canvas_.get(), insertion_marker_->bounds());
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
  return impl_->num_tabs_;
}

int TabStrip::selected_index() const {
  return impl_->selected_tab_ ? impl_->selected_tab_->tab_index() : -1;
}

void TabStrip::DeleteTab(int tab_index) {
  impl_->DeleteTab(tab_index);
}

// On Win8.1
//  SM_CYSIZE = 22
//  SM_CYCAPTION = 23
//  SM_CYEDGE = 1
//  SM_CYSIZEFRAME = 8
Size TabStrip::GetPreferreSize() const {
  const auto font_height = 16;  // must be >= 16 (Small Icon Height)
  const auto button_height = ::GetSystemMetrics(SM_CYSIZE);
  //const auto caption_height = ::GetSystemMetrics(SM_CYCAPTION);
  //const auto edge_height = ::GetSystemMetrics(SM_CYEDGE);
  //const auto frame_height = ::GetSystemMetrics(SM_CYSIZEFRAME);
  return Size(font_height * 40, button_height + k_cyMargin + 34);
}

bool TabStrip::GetTab(int tab_index, TCITEM* tab_data) {
  return impl_->GetTab(tab_index, tab_data);
}

void TabStrip::InsertTab(int new_tab_index, const TCITEM* tab_data) {
  impl_->InsertTab(new_tab_index, tab_data);
}

void TabStrip::SelectTab(int tab_index) {
  impl_->SelectTab(tab_index);
}

void TabStrip::SetIconList(HIMAGELIST icon_list) {
  impl_->SetImageList(icon_list);
  impl_->Redraw();
}

void TabStrip::SetTab(int tab_index, const TCITEM* tab_data) {
  auto const tab = impl_->GetTabFromIndex(tab_index);
  if (!tab)
    return;
  if (tab->SetTab(tab_data))
    tab->Invalidate(impl_->hwnd_);
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
  impl_->bounds_ = bounds();
  impl_->DidCreateNativeWindow();
}

void TabStrip::DidResize() {
  if (!impl_)
    return;
  impl_->bounds_ = bounds();
  impl_->canvas_->Resize(bounds());
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
    impl_->OnLButtonDown(event.location());
}

void TabStrip::OnMouseReleased(const ui::MouseEvent& event) {
  if (event.is_left_button())
    impl_->OnLButtonUp(event.location());
}

void TabStrip::OnPaint(const Rect rect) {
  gfx::Canvas::DrawingScope drawing_scope(*impl_->canvas_);
  impl_->canvas_->set_dirty_rect(rect);
  impl_->Draw(impl_->canvas_.get());
}

}  // namespace views
