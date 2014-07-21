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

static const float kArrowButtonWidth = 20.0f;
static const float kArrowButtonHeight = 20.0f;
// margin between window close button and tab.
static const float kMarginTop = 4;
static const float kMaxTabWidth = 200.0f;
static const float kMinTabWidth = 140.0f;
static const float kTabHeight = 32.0f;

void FillRect(gfx::Canvas* canvas, int x, int y, int cx, int cy) {
  RECT rc;
  rc.left = x;
  rc.right = x + cx;
  rc.top = y;
  rc.bottom = y + cy;
  gfx::Brush brush(canvas, gfx::blackColor());
  canvas->FillRectangle(brush, rc);
}

gfx::Rect RoundBounds(const gfx::RectF& bounds) {
  return gfx::Rect(static_cast<long>(bounds.left),
                   static_cast<long>(bounds.top),
                   static_cast<long>(bounds.right),
                   static_cast<long>(bounds.bottom));
}

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element : public common::Castable {
  DECLARE_CASTABLE_CLASS(Element, Castable);

  public: enum class State {
    Normal,
    Selected,
  };

  private: gfx::RectF bounds_;
  private: HIMAGELIST image_list_;
  private: bool is_hover_;
  private: Element* parent_;
  private: State state_;

  // ctor
  protected: Element(Element* parent);
  public: virtual ~Element() = default;

  protected: gfx::ColorF bgcolor() const;
  public: float bottom() const { return bounds_.bottom; }
  public: const gfx::RectF& bounds() const { return bounds_; }
  public: void set_bounds(const gfx::RectF& new_bounds);
  public: float height() const { return bounds_.height(); }
  public: float left() const { return bounds_.left; }
  public: Element* parent() const { return parent_; }
  public: float right() const { return bounds_.right; }
  public: State state() const { return state_; }
  public: float top() const { return bounds_.top; }
  public: float width() const { return bounds_.width(); }

  // [D]
  public: virtual void Draw(gfx::Canvas* canvas) const = 0;

  // [G]
  public: HIMAGELIST GetImageList() const;

  // [H]
  public: virtual Element* HitTest(const gfx::PointF& point) const;

  // [I]
  public: void Invalidate(HWND hwnd);
  public: bool IsDescendantOf(const Element* other) const;
  public: bool IsHover() const { return is_hover_; }
  public: bool IsSelected() const { return State::Selected == state_; }

  // [S]
  public: bool SetHover(bool new_hover);
  public: void SetImageList(HIMAGELIST hImageList);
  public: Element* SetParent(Element* p);
  public: State SetState(State e);

  // [U]
  protected: virtual void Update();
};

Element::Element(Element* parent)
    : image_list_(nullptr),
      is_hover_(false),
      parent_(parent),
      state_(State::Normal) {
}

gfx::ColorF Element::bgcolor() const {
  if (IsSelected())
      return gfx::whiteColor();
  if (IsHover())
      return gfx::sysColor(COLOR_3DHILIGHT, 0.8);
  return gfx::sysColor(COLOR_3DFACE, 0.5);
}

void Element::set_bounds(const gfx::RectF& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
}

HIMAGELIST Element::GetImageList() const {
  for (const Element* runner = this; runner; runner = runner->parent_) {
    if (auto const image_list = runner->image_list_)
      return image_list;
  }
  return nullptr;
}

Element* Element::HitTest(const gfx::PointF&  point) const {
  return bounds_.Contains(point) ? const_cast<Element*>(this) : nullptr;
}

void Element::Invalidate(HWND hwnd) {
  // TODO(yosi) We should use GFX version of invalidate rectangle.
  auto const bounds = RoundBounds(bounds_);
  ::InvalidateRect(hwnd, &bounds, false);
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

void Element::Update() {
}

//////////////////////////////////////////////////////////////////////
//
// TabStripImpl Design Parameters
//
enum TabStripImplDesignParams {
  k_cxMargin = 0,
  k_cxEdge = 2,
  k_cxBorder = 3,
  k_cxPad = 3,
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

  public: CloseBox(Element* parent);
  public: virtual ~CloseBox() = default;

  // [D]
  private: void DrawXMark(gfx::Canvas* canvas, gfx::ColorF color) const;
  private: gfx::ColorF markColor() const;

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(CloseBox);
};

CloseBox::CloseBox(Element* parent) : Element(parent) {
}

void CloseBox::DrawXMark(gfx::Canvas* canvas, gfx::ColorF color) const {
  gfx::Brush brush(canvas, color);

  auto rc = bounds();
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
      left() + x, top() + y, \
      left() + x + cx, top() + y + cy);

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
  private: gfx::RectF label_bounds_;
  public: base::string16 label_text_;
  public: LPARAM lparam_;
  public: uint32_t state_;
  private: int tab_index_;
  private: views::TabStripDelegate* tab_strip_delegate_;

  public: Tab(views::TabStripDelegate* tab_strip_delegate,
              const TCITEM* pTcItem);
  public: virtual ~Tab() = default;

  public: int tab_index() const { return tab_index_; }
  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  private: void DrawContent(gfx::Canvas* canvas) const;
  private: void DrawIcon(gfx::Canvas* canvas) const;
  public: bool HasCloseBox() const;
  public: bool SetTab(const TCITEM* pTcItem);
  public: void UpdateLayout();

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;
  public: virtual Element* HitTest(const gfx::PointF& point) const override;
  private: void Update() override;

  // ui::Tooltip::ToolDelegate
  private: base::string16 GetTooltipText() override;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

Tab::Tab(views::TabStripDelegate* tab_strip_delegate, const TCITEM* pTcItem)
    : Element(nullptr),
      close_box_(this),
      image_index_(-1),
      tab_index_(0),
      state_(0),
      tab_strip_delegate_(tab_strip_delegate) {
  SetTab(pTcItem);
}

void Tab::DrawContent(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  DrawIcon(canvas);

  // Label Text
  {
    auto rc = bounds();
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
  gfx::Bitmap bitmap(canvas, hIcon);
  auto const icon_size = canvas->AlignToPixel(gfx::SizeF(16, 16));
  auto const icon_offset = canvas->AlignToPixel(gfx::SizeF(-20, 8));
  auto const icon_origin = gfx::PointF(label_bounds_.left, top()) +
                             icon_offset;
  (*canvas)->DrawBitmap(bitmap, gfx::RectF(icon_origin, icon_size));
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

void Tab::UpdateLayout() {
  close_box_.set_bounds(gfx::RectF(
      gfx::PointF(right() - CloseBox::Width, top() + k_cyCloseBoxMargin),
      gfx::SizeF(CloseBox::Width, CloseBox::Height)));

  label_bounds_ = bounds();
  label_bounds_.right = close_box_.right();
  label_bounds_.left += k_cxBorder + k_cxEdge;
  label_bounds_.right -= k_cxBorder + k_cxEdge;
  label_bounds_.top  += 6 + 4;
  label_bounds_.bottom = label_bounds_.top + 12;

  if (image_index_ >= 0)
    label_bounds_.left += 16 + 4;
}

// Element
void Tab::Draw(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  {
    gfx::Brush fillBrush(canvas, bgcolor());
    canvas->FillRectangle(fillBrush, bounds());
    gfx::Brush strokeBrush(canvas, gfx::blackColor());
    canvas->DrawRectangle(strokeBrush, bounds(), 0.2);
  }

  DrawContent(canvas);
  if (HasCloseBox())
    close_box_.Draw(canvas);
  if (!state_)
    return;
  auto const marker_height = 4.0f;
  auto const marker_width = 4.0f;
  DCHECK_GT(width(), marker_width);
  canvas->FillRectangle(
      gfx::Brush(canvas, gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255)),
      gfx::RectF(gfx::PointF(right() - marker_width, top()),
                 gfx::SizeF(marker_width, marker_height)));
}

Element* Tab::HitTest(const gfx::PointF& point) const {
  if (HasCloseBox()) {
    if (auto const hit = close_box_.HitTest(point))
      return hit;
  }
  return Element::HitTest(point);
}

void Tab::Update() {
  UpdateLayout();
}

// ui::Tooltip::ToolDelegate
base::string16 Tab::GetTooltipText() {
  return tab_strip_delegate_->GetTooltipTextForTab(tab_index_);
}

//////////////////////////////////////////////////////////////////////
//
// ArrowButton
//
enum class Direction {
 Down,
 Left,
 Right,
 Up,
};

class ArrowButton final : public Element {
  DECLARE_CASTABLE_CLASS(ArrowButton, Element);

  private: Direction direction_;

  public: ArrowButton(Direction);
  public: virtual ~ArrowButton() = default;

  public: Direction direction() const { return direction_; }

  private: gfx::ColorF ComputeBgColor() const;

  private: void DrawArrow(gfx::Canvas* canvas) const;

  // Element
  private: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(ArrowButton);
};

ArrowButton::ArrowButton(Direction direction)
    : Element(nullptr), direction_(direction) {
}

gfx::ColorF ArrowButton::ComputeBgColor() const {
  if (IsHover())
    return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.3f);
  return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.0f);
}

void ArrowButton::DrawArrow(gfx::Canvas* canvas) const {
  auto const center_x = left() + width() / 2;
  auto const center_y = top() + height() / 2;
  auto const wing_size = width() / 4;
  auto const pen_width = 2.0f;

  auto const alpha = IsHover() ? 1.0f : 0.3f;
  gfx::Brush arrow_brush(canvas, gfx::ColorF(0.0f, 0.0f, 0.0f, alpha));

  float factors[4] = {0.0f};
  switch (direction_) {
    case Direction::Down:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = -1.0f;
      break;
    case Direction::Left:
      factors[0] = 1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Right:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = -1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Up:
      factors[0] = -1.0f;
      factors[1] = 1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
     default:
       NOTREACHED();
   }

  canvas->DrawLine(arrow_brush, center_x + factors[0] * wing_size,
                   center_y + factors[1] * wing_size,
                   center_x, center_y, pen_width);
  canvas->DrawLine(arrow_brush, center_x + factors[2] * wing_size,
                   center_y + factors[3] * wing_size,
                   center_x, center_y, pen_width);
}

// Element
void ArrowButton::Draw(gfx::Canvas* canvas) const {
  if (bounds().empty())
    return;

  gfx::Brush fillBrush(canvas, ComputeBgColor());
  canvas->FillRectangle(fillBrush, bounds());
  DrawArrow(canvas);
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
class TabStrip::TabStripImpl final {
  private: enum class Drag {
    None,
    Tab,
    Start,
  };

  private: typedef std::vector<Tab*> Tabs;

  private: gfx::RectF bounds_;
  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: BOOL composition_enabled;
  private: TabStripDelegate* delegate_;
  private: Tab* dragging_tab_;
  private: Drag drag_state_;
  private: POINT drag_start_point_;
  private: Element* hover_element_;
  private: HWND hwnd_;
  private: Tab* insertion_marker_;
  private: ArrowButton list_button_;
  private: ArrowButton scroll_left_button_;
  private: ArrowButton scroll_right_button_;
  private: Tab* selected_tab_;
  private: bool should_selected_tab_visible_;
  private: Tabs tabs_;
  private: HMENU tab_list_menu_;
  private: gfx::RectF tabs_bounds_;
  private: float tabs_origin_;
  private: ui::Tooltip tooltip_;

  public: TabStripImpl(HWND hwnd, TabStripDelegate* delegate);
  public: ~TabStripImpl();

  public: size_t number_of_tabs() const { return tabs_.size(); }
  public: size_t selected_index() const;

  // [C]
  private: bool ChangeFont();

  // [D]
  private: void DidChangeTabSelection();
  public: void DidCreateNativeWindow();
  public: void DeleteTab(size_t tab_index);
  private: void Draw(gfx::Canvas* canvas) const;
  private: void DropTab(Tab* tab, const POINT& point);

  // [G]
  public: bool GetTab(size_t tab_index, TCITEM* pTcItem) const;

  // [H]
  private: void HandleTabListMenu(POINT point);
  private: Element* HitTest(const gfx::PointF& point) const;

  // [I]
  // Insert a new tab before a tab at |tab_index|.
  public: void InsertTab(size_t tab_index, const TCITEM* pTcItem);

  // [O]
  public: void OnLButtonDown(POINT pt);
  public: void OnLButtonUp(POINT pt);
  public: LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
  public: void OnMouseMove(POINT pt);
  public: LRESULT OnNotify(NMHDR* nmhdr);
  public: void OnPaint(const gfx::Rect& bounds);

  // [R]
  public: void Redraw();
  private: void RenumberTabIndex();

  // [S]
  public: int SelectTab(size_t tab_index);
  private: int SelectTab(Tab* tab);
  public: void SetBounds(const gfx::Rect& bounds);
  public: void SetImageList(HIMAGELIST image_list);
  public: void SetTabData(size_t tab_index, const TCITEM* tab_data);
  private: void StopDrag();

  // [U]
  private: void UpdateBoundsForAllTabs(float tab_width);
  public: void UpdateHover(Element* pHover);
  private: void UpdateLayout();

  DISALLOW_COPY_AND_ASSIGN(TabStripImpl);
};

TabStrip::TabStripImpl::TabStripImpl(HWND hwnd, TabStripDelegate* delegate)
    : canvas_(new gfx::Canvas(gfx::Canvas::DwmSupport::SupportDwm)),
      composition_enabled(false),
      delegate_(delegate),
      dragging_tab_(nullptr),
      drag_state_(Drag::None),
      hover_element_(nullptr),
      hwnd_(hwnd),
      insertion_marker_(nullptr),
      list_button_(Direction::Down),
      scroll_left_button_(Direction::Left),
      scroll_right_button_(Direction::Right),
      selected_tab_(nullptr),
      should_selected_tab_visible_(false),
      tab_list_menu_(nullptr),
      tabs_origin_(0) {
  COM_VERIFY(::DwmIsCompositionEnabled(&composition_enabled));
}

TabStrip::TabStripImpl::~TabStripImpl() {
  if (auto const text_format = canvas_->work<gfx::TextFormat>())
      delete text_format;

  if (tab_list_menu_)
    ::DestroyMenu(tab_list_menu_);
}

size_t TabStrip::TabStripImpl::selected_index() const {
  return selected_tab_ ? selected_tab_->tab_index() : static_cast<size_t>(-1);
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

void TabStrip::TabStripImpl::DeleteTab(size_t tab_index) {
  if (tab_index >= tabs_.size())
    return;
  auto selection_changed = false;
  auto const tab = tabs_[tab_index];
  if (tabs_.size() == 1) {
    tabs_.clear();
    selected_tab_ = nullptr;
    hover_element_ = nullptr;
  } else {
    if (hover_element_ && (hover_element_ == tab ||
                           hover_element_->IsDescendantOf(tab))) {
      hover_element_ = nullptr;
    }
    if (tab == selected_tab_) {
      selection_changed = true;
      selected_tab_ = tabs_[tab_index ? tab_index - 1 : 0];
      selected_tab_->SetState(Element::State::Selected);
    }
    tabs_.erase(tabs_.begin() + static_cast<ptrdiff_t>(tab_index));
    RenumberTabIndex();
  }
  tooltip_.DeleteTool(tab);
  delete tab;

  Redraw();
  if (!selection_changed)
    return;
  DidChangeTabSelection();
}

void TabStrip::TabStripImpl::DidChangeTabSelection() {
  DCHECK(selected_tab_);
  delegate_->DidChangeTabSelection(selected_tab_->tab_index());
}

void TabStrip::TabStripImpl::DidCreateNativeWindow() {
  canvas_->Init(hwnd_);
  ChangeFont();
  tooltip_.Realize(hwnd_);
}

void TabStrip::TabStripImpl::Draw(gfx::Canvas* canvas) const {
  struct Local {
    static void DrawInsertMarker(gfx::Canvas* canvas, Tab* insertion_marker) {
      if (!insertion_marker)
        return;
      auto bounds = insertion_marker->bounds();
      bounds.top += 5;
      bounds.bottom -= 7;
      gfx::Brush brush(canvas, gfx::ColorF::Black);
      for (int w = 1; w <= 7; w += 2) {
        canvas->FillRectangle(brush, gfx::RectF(
            gfx::PointF(bounds.left, bounds.top), gfx::SizeF(w, 1)));
        canvas->FillRectangle(brush, gfx::RectF(
            gfx::PointF(bounds.left, bounds.bottom), gfx::SizeF(w, 1)));
        bounds.top -= 1;
        bounds.left -= 1;
        bounds.bottom += 1;
      }
    }
  };

 auto const bgcolor_alpha = composition_enabled ? 0.0f : 1.0f;
 (*canvas)->Clear(gfx::sysColor(COLOR_3DFACE, bgcolor_alpha));
 if (tabs_.empty())
   return;

  static_cast<const Element&>(scroll_left_button_).Draw(canvas);
  static_cast<const Element&>(list_button_).Draw(canvas);
  static_cast<const Element&>(scroll_right_button_).Draw(canvas);

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_.get(), tabs_bounds_);
  for (auto const tab : tabs_) {
    if (tab->right() < 0)
      continue;
    if (tab->left() > bounds_.right)
      break;
    tab->Draw(canvas);
  }

  Local::DrawInsertMarker(canvas, insertion_marker_);
}

// Send TabDragMsg to window which can handle it.
void TabStrip::TabStripImpl::DropTab(Tab* tab, const POINT& window_point) {
  auto screen_point = window_point;
  if (!::ClientToScreen(hwnd_, &screen_point))
    return;

  for (auto hwnd = ::WindowFromPoint(screen_point); hwnd;
       hwnd = ::GetParent(hwnd)) {
    if (auto const frame = FrameList::instance()->FindFrameByHwnd(hwnd)) {
      static_cast<TabStripDelegate*>(frame)->OnDropTab(tab->lparam_);
      return;
    }
  }

  delegate_->DidThrowTab(tab->lparam_);
}

bool TabStrip::TabStripImpl::GetTab(size_t tab_index, TCITEM* pTcItem) const {
  if (tab_index >= tabs_.size())
    return false;
  auto const tab = tabs_[tab_index];
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

void TabStrip::TabStripImpl::HandleTabListMenu(POINT) {
  POINT menu_origin;
  menu_origin.x = static_cast<int>(list_button_.left());
  menu_origin.y = static_cast<int>(list_button_.bottom());

  ::ClientToScreen(hwnd_, &menu_origin);

  if (!tab_list_menu_)
    tab_list_menu_ = ::CreatePopupMenu();

  // Make Tab List Menu empty
  while (::GetMenuItemCount(tab_list_menu_) > 0) {
    ::DeleteMenu(tab_list_menu_, 0, MF_BYPOSITION);
  }

  // Add Tab name to menu.
  auto last_tab = static_cast<Tab*>(nullptr);
  for (auto const tab : tabs_) {
    auto const flags = tab->IsSelected() ? MF_STRING | MF_CHECKED :
                                           MF_STRING;
    if (last_tab && last_tab->right() < 0 != tab->right() < 0)
      ::AppendMenu(tab_list_menu_, MF_SEPARATOR, 0, nullptr);
    last_tab = tab;
    ::AppendMenu(tab_list_menu_, static_cast<DWORD>(flags),
                 static_cast<DWORD>(tab->tab_index()),
                 tab->label_text_.c_str());
  }

  ::TrackPopupMenuEx(tab_list_menu_, TPM_LEFTALIGN | TPM_TOPALIGN, 
      menu_origin.x, menu_origin.y, hwnd_, nullptr);
}

Element* TabStrip::TabStripImpl::HitTest(const gfx::PointF& point) const {
  if (auto const hit_result = scroll_left_button_.HitTest(point))
    return hit_result;
  if (auto const hit_result = scroll_right_button_.HitTest(point))
    return hit_result;
  if (auto const hit_result = list_button_.HitTest(point))
    return hit_result;

  for (auto const tab : tabs_) {
    if (auto const hit_result = tab->HitTest(point))
      return hit_result;
  }

  return nullptr;
}

void TabStrip::TabStripImpl::InsertTab(size_t tab_index_in,
                                       const TCITEM* pTcItem) {
  auto const tab_index = std::min(tab_index_in, tabs_.size());
  auto const new_tab = new Tab(delegate_, pTcItem);
  tabs_.insert(tabs_.begin() + static_cast<ptrdiff_t>(tab_index), new_tab);
  RenumberTabIndex();
  tooltip_.AddTool(new_tab);
  Redraw();
}

void TabStrip::TabStripImpl::OnLButtonDown(POINT point) {
  auto const element = HitTest(point);
  if (!element)
    return;

  if (auto arrow = element->as<ArrowButton>()) {
    auto const scroll_width = 10.0f;
    switch (arrow->direction()) {
      case Direction::Down:
        HandleTabListMenu(point);
        break;
      case Direction::Left: {
        auto const new_tabs_origin = std::min(tabs_origin_ + scroll_width,
                                              0.0f);
        if (tabs_origin_ == new_tabs_origin)
          break;
        tabs_origin_ = new_tabs_origin;
        should_selected_tab_visible_ = false;
        Redraw();
        break;
      }
      case Direction::Right: {
        auto const min_tabs_origin = tabs_bounds_.width() -
            tabs_.size() * tabs_.front()->width();
        auto const new_tabs_origin = std::max(tabs_origin_ - scroll_width,
                                              min_tabs_origin);
        if (tabs_origin_ == new_tabs_origin)
          break;
        tabs_origin_ = new_tabs_origin;
        should_selected_tab_visible_ = false;
        Redraw();
        break;
      }
    }
    return;
  }

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

  LoadDragTabCursor();

  dragging_tab_ = tab;
  drag_state_ = Drag::Start;
  drag_start_point_ = point;

  ::SetCapture(hwnd_);
}

void TabStrip::TabStripImpl::OnLButtonUp(POINT point) {
  if (!dragging_tab_) {
    auto const element = HitTest(point);
    if (!element)
      return;

    if (element->is<CloseBox>()) {
      if (auto const tab = element->parent()->as<Tab>())
        delegate_->DidClickTabCloseButton(tab->tab_index());
      return;
    }

    return;
  }

  auto const dragging_tab = dragging_tab_;
  auto const insertion_marker = insertion_marker_;
  StopDrag();

  if (!insertion_marker) {
    DropTab(dragging_tab, point);
    return;
  }

  if (dragging_tab != insertion_marker) {
    tabs_.erase(tabs_.begin() + dragging_tab->tab_index());
    tabs_.insert(tabs_.begin() + insertion_marker->tab_index(), dragging_tab);
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
      POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      if (::ScreenToClient(hwnd_, &point)) {
        if (!HitTest(point))
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

void TabStrip::TabStripImpl::OnMouseMove(POINT point) {
  auto const hover_element = HitTest(point);

  if (!dragging_tab_) {
    UpdateHover(hover_element);
  } else {
    if (::GetCapture() != hwnd_) {
      // Someone takes capture. So, we stop dragging.
      StopDrag();
      return;
    }

    if (Drag::Start == drag_state_) {
      if (point.x - drag_start_point_.x >= -5 &&
          point.x - drag_start_point_.x <= 5) {
        return;
      }

      drag_state_ = Drag::Tab;
    }

    // Tab dragging
    auto const insertion_marker = hover_element ? hover_element->as<Tab>() :
                                                  nullptr;
    ::SetCursor(s_hDragTabCursor);
    if (insertion_marker != insertion_marker_)
      ::InvalidateRect(hwnd_, nullptr, false);
    insertion_marker_ = insertion_marker;
  }
}

LRESULT TabStrip::TabStripImpl::OnNotify(NMHDR* nmhdr) {
  tooltip_.OnNotify(nmhdr);
  return 0;
}

void TabStrip::TabStripImpl::OnPaint(const gfx::Rect& bounds) {
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  canvas_->set_dirty_rect(bounds);
  Draw(canvas_.get());
}

void TabStrip::TabStripImpl::Redraw() {
  UpdateLayout();
  ::InvalidateRect(hwnd_, nullptr, false);
}

void TabStrip::TabStripImpl::RenumberTabIndex() {
  auto tab_index = 0;
  for (auto tab : tabs_) {
    tab->set_tab_index(tab_index);
    ++tab_index;
  }
}

int TabStrip::TabStripImpl::SelectTab(size_t tab_index) {
  if (tab_index >= tabs_.size())
    return -1;
  return SelectTab(tabs_[tab_index]);
}

int TabStrip::TabStripImpl::SelectTab(Tab* const tab) {
  should_selected_tab_visible_ = true;
  if (selected_tab_ != tab) {
    if (selected_tab_) {
      selected_tab_->SetState(Element::State::Normal);
      selected_tab_->Invalidate(hwnd_);
    }

    selected_tab_ = tab;

    if (tab) {
      tab->SetState(Element::State::Selected);
      Redraw();
    }

    DidChangeTabSelection();
  }

  return selected_tab_ ? selected_tab_->tab_index() : -1;
}

void TabStrip::TabStripImpl::SetBounds(const gfx::Rect& bounds) {
  bounds_ = gfx::RectF(bounds);
  tabs_bounds_ = bounds_;
  tabs_bounds_.top += ::GetSystemMetrics(SM_CYSIZE) + kMarginTop;
  canvas_->Resize(bounds);
  Redraw();
}

void TabStrip::TabStripImpl::SetImageList(HIMAGELIST image_list) {
  for (auto const tab : tabs_) {
    tab->SetImageList(image_list);
  }
}

void TabStrip::TabStripImpl::SetTabData(size_t tab_index,
                                        const TCITEM* tab_data) {
  if (tab_index >= tabs_.size())
    return;
  auto const tab = tabs_[tab_index];
  if (!tab->SetTab(tab_data))
    return;
  tab->Invalidate(hwnd_);
}

void TabStrip::TabStripImpl::StopDrag() {
  drag_state_ = Drag::None;
  dragging_tab_ = nullptr;
  insertion_marker_ = nullptr;

  ::ReleaseCapture();
  ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

void TabStrip::TabStripImpl::UpdateBoundsForAllTabs(float tab_width) {
  auto origin = gfx::PointF(tabs_origin_ + tabs_bounds_.left,
                              tabs_bounds_.top);
  auto const tab_size = gfx::SizeF(tab_width,
                                   tabs_bounds_.bottom - origin.y);
  for (auto const tab : tabs_){
    tab->set_bounds(gfx::RectF(origin, tab_size));
    tab->UpdateLayout();
    origin.x += tab_width;
    tooltip_.SetToolBounds(tab, RoundBounds(bounds_.Intersect(tab->bounds())));
  }
}

void TabStrip::TabStripImpl::UpdateHover(Element* hover_element) {
    if (hover_element_ == hover_element)
      return;

  if (hover_element_) {
    if (!hover_element || !hover_element->is<CloseBox>() ||
        hover_element->parent() != hover_element_) {
      if (hover_element_->is<CloseBox>()) {
        hover_element_->SetHover(false);
        hover_element_ = hover_element_->parent();
      }
      hover_element_->SetHover(false);
      hover_element_->Invalidate(hwnd_);
    }
  }

  hover_element_ = hover_element;
  if (hover_element_) {
    hover_element_->SetHover(true);
    hover_element_->Invalidate(hwnd_);
  }
}

void TabStrip::TabStripImpl::UpdateLayout() {
  if (tabs_.empty()) {
    tabs_origin_ = 0;
    return;
  }

  auto const tab_width = std::min(
      std::max(bounds_.width() / tabs_.size(), kMinTabWidth), kMaxTabWidth);

  if (tabs_.size() * tab_width < bounds_.width()) {
    scroll_left_button_.set_bounds(gfx::RectF());
    scroll_right_button_.set_bounds(gfx::RectF());
    list_button_.set_bounds(gfx::RectF());
    tabs_bounds_.left = bounds_.left;
    tabs_bounds_.right = bounds_.right;
    tabs_origin_ = 0;
  } else {
    auto const button_top = tabs_bounds_.top;
    auto const button_height = tabs_bounds_.height();
    scroll_left_button_.set_bounds(gfx::RectF(
        gfx::PointF(bounds_.left, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    list_button_.set_bounds(gfx::RectF(
        gfx::PointF(bounds_.right - kArrowButtonWidth, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    scroll_right_button_.set_bounds(gfx::RectF(
        gfx::PointF(list_button_.left() - kArrowButtonWidth, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    tabs_bounds_.left = scroll_left_button_.right();
    tabs_bounds_.right = scroll_right_button_.left();
  }

  UpdateBoundsForAllTabs(tab_width);

  if (!selected_tab_ || !should_selected_tab_visible_ ||
      tabs_bounds_.Contains(selected_tab_->bounds())) {
    return;
  }

  // Make selected tab visible.
  if ((selected_tab_->tab_index() + 1) * tab_width < tabs_bounds_.width()) {
    tabs_origin_ = 0.0f;
  } else if ((tabs_.size() - selected_tab_->tab_index()) * tab_width <
             tabs_bounds_.width()) {
    tabs_origin_ = -(tabs_.size() * tab_width) + tabs_bounds_.width();
  } else if (selected_tab_->left() >= tabs_bounds_.left &&
             selected_tab_->left() < tabs_bounds_.right) {
    tabs_origin_ -= selected_tab_->right() - tabs_bounds_.right +
                    tab_width / 3;
  } else {
    tabs_origin_ += -selected_tab_->left() + tab_width / 3;
  }
  UpdateBoundsForAllTabs(tab_width);
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
  return static_cast<int>(impl_->number_of_tabs());
}

int TabStrip::selected_index() const {
  return static_cast<int>(impl_->selected_index());
}

void TabStrip::DeleteTab(int tab_index) {
  impl_->DeleteTab(static_cast<size_t>(tab_index));
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
  return Size(font_height * 40,
              static_cast<int>(button_height + kMarginTop + 34));
}

bool TabStrip::GetTab(int tab_index, TCITEM* tab_data) {
  return impl_->GetTab(static_cast<size_t>(tab_index), tab_data);
}

void TabStrip::InsertTab(int new_tab_index, const TCITEM* tab_data) {
  impl_->InsertTab(static_cast<size_t>(new_tab_index), tab_data);
}

void TabStrip::SelectTab(int tab_index) {
  impl_->SelectTab(static_cast<size_t>(tab_index));
}

void TabStrip::SetIconList(HIMAGELIST icon_list) {
  impl_->SetImageList(icon_list);
  impl_->Redraw();
}

void TabStrip::SetTab(int tab_index, const TCITEM* tab_data) {
  impl_->SetTabData(static_cast<size_t>(tab_index), tab_data);
}

// ui::Widget
void TabStrip::CreateNativeWindow() const {
  native_window()->CreateWindowEx(
      0, WS_CHILD | WS_VISIBLE, L"TabStrip", parent_node()->AssociatedHwnd(),
      bounds().origin(),
      bounds().size());
}

void TabStrip::DidCreateNativeWindow() {
  impl_.reset(new TabStripImpl(*native_window(), delegate_));
  impl_->DidCreateNativeWindow();
}

void TabStrip::DidResize() {
  if (!impl_)
    return;
  impl_->SetBounds(bounds());
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

void TabStrip::OnPaint(const Rect bounds) {
  impl_->OnPaint(bounds);
}

}  // namespace views
