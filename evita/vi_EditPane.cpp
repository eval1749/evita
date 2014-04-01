#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Edit Pane
// listener/winapp/vi_EditPane.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_EditPane.cpp#3 $
//
#define DEBUG_REDRAW 0
#define DEBUG_RESIZE 0
#define DEBUG_SPLIT 0
#include "evita/vi_EditPane.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/editor/application.h"
#include "evita/resource.h"
#include "evita/ui/events/event.h"
#include "evita/ui/widget_ostream.h"
#include "evita/views/content_window.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"

extern HINSTANCE g_hInstance;

struct EditPane::HitTestResult {
  enum Type {
    None,
    HScrollBar,
    HSplitter,
    HSplitterBig,
    VScrollBar,
    VSplitter,
    VSplitterBig,
    Window,
  };

  Box* box;
  Type type;

  HitTestResult() : box(nullptr), type(None) {
  }

  HitTestResult(Type type, const Box& box)
      : box(const_cast<Box*>(&box)), type(type) {
    ASSERT(type != None);
  }

  views::ContentWindow* window() const;
};

class EditPane::Box : public DoubleLinkedNode_<EditPane::Box>,
                      public base::RefCounted<EditPane::Box> {
    protected: EditPane* edit_pane_;
    private: bool is_removed_;
    private: LayoutBox* outer_;
    private: Rect rect_;
    protected: Box(EditPane*, LayoutBox*);
    public: virtual ~Box();
    public: bool is_removed() const { return is_removed_; }
    public: virtual Box* first_child() const { return nullptr; }
    public: Box* next_sibling() const { return GetNext(); }
    public: Box* previous_sibling() const { return GetPrev(); }
    public: LayoutBox* outer() const { return outer_; }
    public: const gfx::Rect& rect() const { return rect_; }
    public: Rect& rect() { return rect_; }
    public: void set_outer(LayoutBox& outer) { outer_ = &outer; }
    public: virtual uint CountLeafBox() const = 0;
    public: virtual void Destroy() = 0;
    public: virtual void DrawSplitters(const gfx::Graphics&) { }
    public: virtual LeafBox* FindLeafBoxFromWidget(
        const Widget&) const = 0;
    public: virtual LeafBox* GetActiveLeafBox() const = 0;
    public: virtual LeafBox* GetFirstLeafBox() const = 0;

    public: virtual HitTestResult HitTest(Point) const = 0;

    public: virtual bool IsLeafBox() const = 0;

    public: virtual void Realize(EditPane*, const gfx::Rect&);
    public: virtual void Redraw() const {}
    public: void Removed();

    public: virtual void SetRect(const gfx::Rect&);

    DISALLOW_COPY_AND_ASSIGN(Box);
};

class EditPane::LayoutBox : public EditPane::Box {
  protected: typedef DoubleLinkedList_<Box> BoxList;
  protected: BoxList boxes_;
  protected: LayoutBox(EditPane*, LayoutBox*);
  public: virtual ~LayoutBox();
  public: virtual Box* first_child() const override final {
    return boxes_.GetFirst();
  }
  public: void Add(Box& box);
  public: virtual uint CountLeafBox() const override final;
  public: virtual void Destroy() override final;
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) = 0;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) = 0;
  public: virtual LeafBox* GetActiveLeafBox() const override final;
  public: virtual LeafBox* GetFirstLeafBox() const override final;
  public: virtual LeafBox* FindLeafBoxFromWidget(
      const Widget&) const override final;

  public: virtual bool IsLeafBox() const override final { return false; }
  public: bool IsSingle() const;
  public: virtual bool IsVerticalLayoutBox() const = 0;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void Redraw() const override final;
  public: void RemoveBox(Box&);
  public: void Replace(Box&, Box&);
  public: virtual void Split(Box* ref_box, Window* new_window,
                             int new_box_size) = 0;
  public: virtual void StopSplitter(const gfx::Point&, Box&) = 0;

  // [U]
  public: void UpdateSplitters();

  DISALLOW_COPY_AND_ASSIGN(LayoutBox);
};

class EditPane::LeafBox final : public EditPane::Box {
  private: Window* m_pWindow;

  public: LeafBox(EditPane* edit_pane, LayoutBox* outer, Window* pWindow)
    : Box(edit_pane, outer),
      m_pWindow(pWindow) {
  }

  public: virtual ~LeafBox();

  // [C]
  public: virtual uint CountLeafBox() const  override final { return 1; }

  // [D]
  public: virtual void Destroy() override;
  public: void DetachWindow();
  private: virtual void DrawSplitters(const gfx::Graphics&) override;

  public: void EnsureInHorizontalLayoutBox();
  public: void EnsureInVerticalLayoutBox();

  // [G]
  public: virtual LeafBox* GetActiveLeafBox() const override;
  public: virtual LeafBox* GetFirstLeafBox() const override;
  public: virtual LeafBox* FindLeafBoxFromWidget(
      const Widget&) const override;
  public: Window* GetWindow() const { return m_pWindow; }

  // [H]
  private: bool HasSibling() const { return GetNext() || GetPrev(); }
  public: virtual HitTestResult HitTest(Point) const override;

  // [I]
  public: virtual bool IsLeafBox() const override final { return true; }

  // [R]
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void Redraw() const override;
  public: void ReplaceWindow(Window* window);

  // [S]
  public: virtual void SetRect(const gfx::Rect&) override;

  DISALLOW_COPY_AND_ASSIGN(LeafBox);
};

class EditPane::HorizontalLayoutBox final : public EditPane::LayoutBox {
  public: HorizontalLayoutBox(EditPane*, LayoutBox*);
  public: virtual ~HorizontalLayoutBox();
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) override;
  public: virtual HitTestResult HitTest(Point) const override;
  public: virtual void DrawSplitters(const gfx::Graphics&) override;
  public: virtual bool IsVerticalLayoutBox() const override;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) override;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void SetRect(const gfx::Rect&) override;
  private: virtual void Split(Box* ref_box, Window* new_window,
                              int new_box_size) override;
  public: virtual void StopSplitter(const gfx::Point&, Box&) override;
  DISALLOW_COPY_AND_ASSIGN(HorizontalLayoutBox);
};

class EditPane::VerticalLayoutBox final : public LayoutBox {
  public: VerticalLayoutBox(EditPane*, LayoutBox*);
  public: virtual ~VerticalLayoutBox();
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) override;
  public: virtual HitTestResult HitTest(Point) const override;
  public: virtual void DrawSplitters(const gfx::Graphics&) override;
  public: virtual bool IsVerticalLayoutBox() const override;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) override;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void SetRect(const gfx::Rect&) override;
  private: virtual void Split(Box* ref_box, Window* new_window,
                              int new_box_size) override;
  public: virtual void StopSplitter(const gfx::Point&, Box&) override;
  DISALLOW_COPY_AND_ASSIGN(VerticalLayoutBox);
};

namespace {
class StockCursor {
  private: HCURSOR hCursor_;
  private: HINSTANCE hInstance_;
  private: const char16* id_;
  public: explicit StockCursor(int id)
    : StockCursor(g_hInstance, MAKEINTRESOURCE(id)) {
  }
  public: explicit StockCursor(const char16* id)
    : StockCursor(nullptr, id) {
  }
  private: StockCursor(HINSTANCE instance, const char16* id)
      : hCursor_(nullptr), hInstance_(instance), id_(id) {
  }
  public: operator HCURSOR() {
    if (!hCursor_) {
      hCursor_ = ::LoadCursor(hInstance_, id_);
      ASSERT(hCursor_);
    }
    return hCursor_;
  }
  DISALLOW_COPY_AND_ASSIGN(StockCursor);
};

void DrawSplitter(const gfx::Graphics& gfx, RECT* prc,
                         uint /*grfFlag*/) {
  auto rc = *prc;
  //gfx::Brush fillBrush(gfx, gfx::sysColor(COLOR_3DFACE));
  gfx::Brush fillBrush(gfx, gfx::ColorF(gfx::ColorF::Pink));
  gfx.FillRectangle(fillBrush, rc);
  //::DrawEdge(gfx, &rc, EDGE_RAISED, grfFlag);
}

} // namesapce

//
// EditPane
//

EditPane::Box::Box(EditPane* edit_pane, LayoutBox* outer)
    : edit_pane_(edit_pane),
      is_removed_(false),
      outer_(outer) {}

EditPane::Box::~Box() {
  ASSERT(is_removed_);
  ASSERT(!outer_);
  ASSERT(!GetNext());
  ASSERT(!GetPrev());
}

void EditPane::Box::Realize(EditPane*, const gfx::Rect& rect) {
  rect_ = rect;
}

void EditPane::Box::Removed() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!is_removed());
  is_removed_ = true;
  outer_ = nullptr;
}

void EditPane::Box::SetRect(const gfx::Rect& rect) {
  rect_ = rect;
}

// HorizontalLayoutBox
EditPane::HorizontalLayoutBox::HorizontalLayoutBox(EditPane* edit_pane,
                                                   LayoutBox* outer)
    : LayoutBox(edit_pane, outer) {
}

EditPane::HorizontalLayoutBox::~HorizontalLayoutBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
}

void EditPane::HorizontalLayoutBox::DidRemoveBox(
    Box* const pAbove,
    Box* const pBelow,
    const gfx::Rect& rc) {
  if (pAbove) {
    // Extend pane above.
    RECT rect = pAbove->rect();
    rect.right = rc.right;
    pAbove->SetRect(rect);
  } else if (pBelow) {
    // Extend pane below.
    RECT rect = pBelow->rect();
    rect.left = rc.left;
    pBelow->SetRect(rect);
  }
}

void EditPane::HorizontalLayoutBox::DrawSplitters(const gfx::Graphics& gfx) {
  if (boxes_.GetFirst() == boxes_.GetLast()) {
    return;
  }

  auto rc = rect();
  foreach (BoxList::Enum, it, boxes_) {
    auto const box = it.Get();
    box->DrawSplitters(gfx);
    if (auto const right_box = box->GetPrev()) {
      rc.left = right_box->rect().right;
      rc.right = box->rect().left;
      DrawSplitter(gfx, &rc, BF_LEFT | BF_RIGHT);
    }
  }
}

EditPane::HitTestResult EditPane::HorizontalLayoutBox::HitTest(
    Point pt) const {
  if (!::PtInRect(&rect(), pt)) {
    return HitTestResult();
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const result = it->HitTest(pt);
    if (result.type != HitTestResult::None) {
      return result;
    }

    if (auto const left_box = it->GetPrev()) {
      RECT splitterRect;
      splitterRect.top = rect().top;
      splitterRect.bottom = rect().bottom;
      splitterRect.left = left_box->rect().right;
      splitterRect.right = it->rect().left;
      if (::PtInRect(&splitterRect, pt)) {
        return HitTestResult(HitTestResult::HSplitter, *it.Get());
      }
    }
  }

  return HitTestResult();
}

bool EditPane::HorizontalLayoutBox::IsVerticalLayoutBox() const {
  return false;
}

void EditPane::HorizontalLayoutBox::MoveSplitter(
    const gfx::Point& pt,
    Box& right_box) {
  if (!right_box.GetPrev())
    return;
  auto& left_box = *right_box.GetPrev();
  if (pt.x - left_box.rect().left <= 0) {
    // Above box is too small.
  } else if (right_box.rect().right - pt.x <= k_cxMinBox) {
    // Below box is too small.
  } else {
    left_box.rect().right = pt.x;
    right_box.rect().left = pt.x + k_cxSplitter;
    left_box.SetRect(left_box.rect());
    right_box.SetRect(right_box.rect());
  }

  UpdateSplitters();
}

void EditPane::HorizontalLayoutBox::Realize(
    EditPane* edit_pane,
    const gfx::Rect& rect) {
  LayoutBox::Realize(edit_pane, rect);

  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->Realize(edit_pane, rect);
    return;
  }

  auto const width = rect.right - rect.left;
  auto const content_width = width - k_cxSplitter * (num_boxes - 1);
  auto const box_width = content_width / num_boxes;
  RECT elemRect(rect);
  foreach (BoxList::Enum, it, boxes_) {
    elemRect.right = rect.left + box_width;
    it->Realize(edit_pane, elemRect);
    elemRect.left = elemRect.right + k_cxSplitter;
  }
}

void EditPane::HorizontalLayoutBox::SetRect(const gfx::Rect& newRect) {
  RECT rcOld = rect();
  LayoutBox::SetRect(newRect);
  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->SetRect(newRect);
    return;
  }

  auto const cxNewPane = rect().right  - rect().left;
  auto const cxOldPane = rcOld.right - rcOld.left;

  if (!cxOldPane) {
    auto const cBoxes = boxes_.Count();
    if (!cBoxes) {
      return;
    }

    auto const cxNewWin = cxNewPane / cBoxes;
    auto xBox = rect().left;
    auto cxSplitter = 0;
    auto pBox = static_cast<Box*>(nullptr);
    foreach (BoxList::Enum, oEnum, boxes_) {
      pBox = oEnum.Get();
      auto const prc = &pBox->rect();
      xBox += cxSplitter;
      prc->left = xBox;
      xBox += cxNewWin;
      prc->right = xBox;
      cxSplitter = k_cxSplitter;
    }

    if (pBox) {
      pBox->rect().right = rect().right;
    }
  } else {
    scoped_refptr<LayoutBox> protect(this);

    tryAgain:
      auto xBox = rect().left;
      auto cxSplitter = 0;
      auto pBox = static_cast<Box*>(nullptr);
      foreach (BoxList::Enum, oEnum, boxes_) {
        pBox = oEnum.Get();
        auto const prc = &pBox->rect();
        auto const cxOldWin = prc->right - prc->left;
        auto const cxNewWin = cxNewPane * cxOldWin / cxOldPane;
        if (cxNewWin < k_cxMinBox) {
          pBox->Destroy();
          if (is_removed())
            return;
          goto tryAgain;
        }
        xBox += cxSplitter;
        prc->left = xBox;
        xBox += cxNewWin;
        prc->right = xBox;
        cxSplitter = k_cxSplitter;
      }

      if (!pBox) {
        return;
      }
      pBox->rect().right = rect().right;
  }

  foreach (BoxList::Enum, oEnum, boxes_) {
    auto const pBox = oEnum.Get();
    auto newRect = pBox->rect();
    newRect.top = rect().top;
    newRect.bottom = rect().bottom;
    pBox->SetRect(newRect);
  }
}

void EditPane::HorizontalLayoutBox::Split(Box* left_box,
                                          Window* new_right_window,
                                          int new_right_width) {
  DCHECK(!new_right_window->is_realized());
  edit_pane_->AppendChild(new_right_window);

  auto right_box = new LeafBox(edit_pane_, this, new_right_window);
  boxes_.InsertAfter(right_box, left_box);
  right_box->AddRef();

  auto left_box_rect = left_box->rect();
  auto right_box_rect = left_box->rect();
  right_box_rect.left = right_box_rect.right - new_right_width;
  left_box_rect.right = right_box_rect.left - k_cxSplitter;
  left_box->SetRect(left_box_rect);
  right_box->Realize(edit_pane_, right_box_rect);
  edit_pane_->SchedulePaintInRect(Rect(left_box_rect.right, rect().top,
                                       right_box_rect.left, rect().bottom));
}

void EditPane::HorizontalLayoutBox::StopSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  if (!below_box.GetPrev()) {
    return;
  }

  scoped_refptr<LayoutBox> protect(this);
  auto& above_box = *below_box.GetPrev();
  auto const cxMin = k_cxMinBox;
  if (pt.x - above_box.rect().left < cxMin) {
    below_box.rect().left = above_box.rect().left;
    above_box.Destroy();
    below_box.SetRect(below_box.rect());
    UpdateSplitters();
  } else if (below_box.rect().right - pt.x < k_cxMinBox) {
    above_box.rect().right = below_box.rect().right;
    below_box.Destroy();
    above_box.SetRect(above_box.rect());
    UpdateSplitters();
  }
}

// LayoutBox
EditPane::LayoutBox::LayoutBox(EditPane* edit_pane, LayoutBox* outer)
    : Box(edit_pane, outer) {
}

EditPane::LayoutBox::~LayoutBox() {
  ASSERT(boxes_.IsEmpty());
}

void EditPane::LayoutBox::Add(Box& box) {
  ASSERT(!is_removed());
  boxes_.Append(&box);
  box.AddRef();
}

EditPane::LeafBox* EditPane::LayoutBox::GetActiveLeafBox() const {
  ASSERT(!is_removed());
  class Local {
    public: static LeafBox* SelectActiveBox(LeafBox* box1, LeafBox* box2) {
      return box1 && box2
          ? activeTick(*box1) > activeTick(*box2) ? box1 : box2
          : box1 ? box1 : box2;
    }

    private: static uint activeTick(const LeafBox& box) {
      auto& window = *box.GetWindow();
      return window.is_shown() ? window.active_tick() : 0u;
    }
  };

  auto candiate = static_cast<LeafBox*>(nullptr);
  for (auto& box: boxes_) {
    auto const other = box.GetActiveLeafBox();
    //if (other && other->GetWindow()->is_shown())
    candiate = Local::SelectActiveBox(candiate, other);
  }
  return candiate;
}

EditPane::LeafBox* EditPane::LayoutBox::GetFirstLeafBox() const {
  ASSERT(!is_removed());
  return boxes_.GetFirst() ? boxes_.GetFirst()->GetFirstLeafBox() : nullptr;
}

uint EditPane::LayoutBox::CountLeafBox() const {
  ASSERT(!is_removed());
  auto count = 0u;
  foreach (BoxList::Enum, it, boxes_) {
    count += it->CountLeafBox();
  }
  return count;
}

void EditPane::LayoutBox::Destroy() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!is_removed());
  scoped_refptr<LayoutBox> protect(this);
  while (auto const box = boxes_.GetFirst()) {
    box->Destroy();
  }
}

EditPane::LeafBox* EditPane::LayoutBox::FindLeafBoxFromWidget(
    const Widget& window) const {
  ASSERT(!is_removed());
  foreach (BoxList::Enum, it, boxes_) {
    if (auto const box = it->FindLeafBoxFromWidget(window)) {
      return box;
    }
  }
  return nullptr;
}

bool EditPane::LayoutBox::IsSingle() const {
  ASSERT(!is_removed());
  return !boxes_.IsEmpty() && boxes_.GetFirst() == boxes_.GetLast();
}

void EditPane::LayoutBox::Realize(EditPane* edit_pane, const gfx::Rect& rect) {
  ASSERT(!is_removed());
  Box::Realize(edit_pane, rect);
}

void EditPane::LayoutBox::Redraw() const {
  for (auto& box : boxes_) {
    box.Redraw();
  }
}

void EditPane::LayoutBox::RemoveBox(Box& box) {
  ASSERT(!is_removed());
  auto const pAbove = box.GetPrev();
  auto const pBelow = box.GetNext();
  boxes_.Delete(&box);
  auto const rc = box.rect();
  box.Removed();
  box.Release();
  DidRemoveBox(pAbove, pBelow, rc);

  if (!outer()) {
    return;
  }

  if (boxes_.IsEmpty()) {
    outer()->RemoveBox(*this);
    return;
  }

  if (boxes_.GetFirst() != boxes_.GetLast()) {
    return;
  }

  auto& first_box = *boxes_.GetFirst();
  boxes_.Delete(&first_box);
  auto const outer = this->outer();
  Removed();
  outer->Replace(first_box, *this);
  first_box.Release();
  return;
}

void EditPane::LayoutBox::Replace(Box& new_box, Box& old_box) {
  ASSERT(!is_removed());
  boxes_.InsertBefore(&new_box, &old_box);
  new_box.set_outer(*this);
  new_box.AddRef();
  boxes_.Delete(&old_box);
  old_box.Release();
}

void EditPane::LayoutBox::UpdateSplitters() {
  if (is_removed() || !edit_pane_->is_shown())
    return;
  auto& gfx = edit_pane_->frame().gfx();
  gfx::Graphics::DrawingScope drawing_scope(gfx);
  DrawSplitters(gfx);
}

// HitTestResult
EditPane::Window* EditPane::HitTestResult::window() const {
  return static_cast<LeafBox*>(box)->GetWindow();
}

// LeafBox
EditPane::LeafBox::~LeafBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!m_pWindow);
}

void EditPane::LeafBox::Destroy() {
  GetWindow()->DestroyWidget();
}

void EditPane::LeafBox::DetachWindow() {
  m_pWindow = nullptr;
}

void EditPane::LeafBox::DrawSplitters(const gfx::Graphics& gfx) {
  m_pWindow->OnDraw(const_cast<gfx::Graphics*>(&gfx));
}

void EditPane::LeafBox::EnsureInHorizontalLayoutBox() {
  if (!outer()->IsVerticalLayoutBox()) {
    return;
  }

  auto& layout_box = *new HorizontalLayoutBox(edit_pane_, outer());
  scoped_refptr<LeafBox> protect(this);
  outer()->Replace(layout_box, *this);
  layout_box.Realize(edit_pane_, rect());
  layout_box.Add(*this);
  set_outer(layout_box);
}

void EditPane::LeafBox::EnsureInVerticalLayoutBox() {
  if (outer()->IsVerticalLayoutBox()) {
    return;
  }

  auto& layout_box = *new VerticalLayoutBox(edit_pane_, outer());
  scoped_refptr<LeafBox> protect(this);
  outer()->Replace(layout_box, *this);
  layout_box.Realize(edit_pane_, rect());
  layout_box.Add(*this);
  set_outer(layout_box);
}

EditPane::LeafBox* EditPane::LeafBox::GetActiveLeafBox() const {
  return const_cast<LeafBox*>(this);
}

EditPane::LeafBox* EditPane::LeafBox::GetFirstLeafBox() const {
  return const_cast<LeafBox*>(this);
}

EditPane::LeafBox* EditPane::LeafBox::FindLeafBoxFromWidget(
    const Widget& window) const {
  return window == m_pWindow ? const_cast<LeafBox*>(this) : nullptr;
}

EditPane::HitTestResult EditPane::LeafBox::HitTest(Point pt) const {
  if (!::PtInRect(&rect(), pt))
    return HitTestResult();

  auto const cxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
  if (pt.x < rect().right - cxVScroll)
    return HitTestResult(HitTestResult::Window, *this);

  if (!HasSibling() && pt.y < rect().top + k_cySplitterBig)
    return HitTestResult(HitTestResult::VSplitterBig, *this);

  return HitTestResult();
}

void EditPane::LeafBox::Realize(EditPane* edit_pane, const gfx::Rect& rect) {
  Box::Realize(edit_pane, rect);
  m_pWindow->Realize(rect);
  m_pWindow->Show();
  SetRect(rect);
}

void EditPane::LeafBox::Redraw() const {
  m_pWindow->Redraw();
}

void EditPane::LeafBox::ReplaceWindow(Window* window) {
  DCHECK(!window->parent_node());
  DCHECK(!window->is_realized());
  auto const previous_window = m_pWindow;
  m_pWindow = window;
  edit_pane_->AppendChild(window);
  Realize(edit_pane_, rect());
  previous_window->DestroyWidget();
}

void EditPane::LeafBox::SetRect(const gfx::Rect& rect) {
  Box::SetRect(rect);
  m_pWindow->ResizeTo(rect);
}

EditPane::VerticalLayoutBox::VerticalLayoutBox(EditPane* edit_pane,
                                               LayoutBox* outer)
    : LayoutBox(edit_pane, outer) {}

EditPane::VerticalLayoutBox::~VerticalLayoutBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
}

void EditPane::VerticalLayoutBox::DidRemoveBox(
    Box* const pAbove,
    Box* const pBelow,
    const gfx::Rect& rc) {
  if (pAbove) {
    // Extend pane above.
    RECT rect = pAbove->rect();
    rect.bottom = rc.bottom;
    pAbove->SetRect(rect);

  } else if (pBelow) {
    // Extend pane below.
    RECT rect = pBelow->rect();
    rect.top = rc.top;
    pBelow->SetRect(rect);
  }
}

void EditPane::VerticalLayoutBox::DrawSplitters(const gfx::Graphics& gfx) {
  auto rc = rect();

  if (boxes_.GetFirst() == boxes_.GetLast()) {
    auto const cxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
    rc.left = rc.right - cxVScroll;
    rc.bottom = rc.top + k_cySplitterBig;
    DrawSplitter(gfx, &rc, BF_RECT);
    boxes_.GetFirst()->DrawSplitters(gfx);
    return;
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const box = it.Get();
    box->DrawSplitters(gfx);
    if (auto const above_box = box->GetPrev()) {
      rc.top = above_box->rect().bottom;
      rc.bottom = box->rect().top;
      DrawSplitter(gfx, &rc, BF_TOP | BF_BOTTOM);
    }
  }
}

EditPane::HitTestResult EditPane::VerticalLayoutBox::HitTest(
    Point pt) const {
  if (!::PtInRect(&rect(), pt)) {
    return HitTestResult();
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const result = it->HitTest(pt);
    if (result.type != HitTestResult::None) {
      return result;
    }

    if (auto const above_box = it->GetPrev()) {
      RECT splitterRect;
      splitterRect.left = rect().left;
      splitterRect.right = rect().right;
      splitterRect.top = above_box->rect().bottom;
      splitterRect.bottom = it->rect().top;
      if (::PtInRect(&splitterRect, pt)) {
        return HitTestResult(HitTestResult::VSplitter, *it.Get());
      }
    }
  }

  return HitTestResult();
}

bool EditPane::VerticalLayoutBox::IsVerticalLayoutBox() const {
  return true;
}

void EditPane::VerticalLayoutBox::MoveSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  auto const pBelow = &below_box;
  auto const pAbove = pBelow->GetPrev();
  if (!pAbove)
    return;

  if (pt.y - pAbove->rect().top <= 0) {
    // Above box is too small.
  } else if (pBelow->rect().bottom - (pt.y + k_cySplitter) <= 0) {
    // Below box is too small.
  } else {
    pAbove->rect().bottom = pt.y;
    pBelow->rect().top = pt.y + k_cySplitter;
    pAbove->SetRect(pAbove->rect());
    pBelow->SetRect(pBelow->rect());
  }

  UpdateSplitters();
}

void EditPane::VerticalLayoutBox::Realize(
    EditPane* edit_pane,
    const gfx::Rect& rect) {
  LayoutBox::Realize(edit_pane, rect);

  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->Realize(edit_pane, rect);
    return;
  }

  auto const height = rect.bottom - rect.top;
  auto const content_height = height - k_cySplitter * (num_boxes - 1);
  auto const box_height = content_height / num_boxes;
  RECT elemRect(rect);
  foreach (BoxList::Enum, it, boxes_) {
    elemRect.bottom = rect.top + box_height;
    it->Realize(edit_pane, elemRect);
    elemRect.top = elemRect.bottom + k_cySplitter;
  }
}

void EditPane::VerticalLayoutBox::SetRect(const gfx::Rect& newRect) {
  RECT rcOld = rect();
  LayoutBox::SetRect(newRect);
  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->SetRect(newRect);
    return;
  }

  auto const cyNewPane = rect().bottom  - rect().top;
  auto const cyOldPane = rcOld.bottom - rcOld.top;

  if (!cyOldPane) {
    auto const cBoxes = boxes_.Count();
    if (!cBoxes) {
      return;
    }

    auto const cyNewWin = cyNewPane / cBoxes;
    auto yBox = rect().top;
    auto cySplitter = 0;
    auto pBox = static_cast<Box*>(nullptr);
    foreach (BoxList::Enum, oEnum, boxes_) {
      pBox = oEnum.Get();
      auto const prc = &pBox->rect();
      yBox += cySplitter;
      prc->top = yBox;
      yBox += cyNewWin;
      prc->bottom = yBox;
      cySplitter = k_cySplitter;
    }

    if (pBox) {
      pBox->rect().bottom = rect().bottom;
    }
  } else {
    scoped_refptr<LayoutBox> protect(this);
    tryAgain:
      auto yBox = rect().top;
      auto cySplitter = 0;
      auto pBox = static_cast<Box*>(nullptr);
      foreach (BoxList::Enum, oEnum, boxes_) {
        pBox = oEnum.Get();
        auto const prc = &pBox->rect();
        auto const cyOldWin = prc->bottom - prc->top;
        auto const cyNewWin = cyNewPane * cyOldWin / cyOldPane;
        if (cyNewWin < k_cyMinBox) {
          pBox->Destroy();
          if (is_removed()) {
            return;
          }
          goto tryAgain;
        }
        yBox += cySplitter;
        prc->top = yBox;
        yBox += cyNewWin;
        prc->bottom = yBox;
        cySplitter = k_cySplitter;
      }

      if (!pBox) {
        return;
      }
      pBox->rect().bottom = rect().bottom;
  }

  foreach (BoxList::Enum, oEnum, boxes_) {
    auto const pBox = oEnum.Get();
    auto newRect = pBox->rect();
    newRect.left = rect().left;
    newRect.right = rect().right;
    pBox->SetRect(newRect);
  }
}

void EditPane::VerticalLayoutBox::Split(Box* above_box,
                                        Window* new_below_window,
                                        int new_below_height) {
  DCHECK(!new_below_window->is_realized());
  edit_pane_->AppendChild(new_below_window);

  auto below_box = new LeafBox(edit_pane_, this, new_below_window);
  boxes_.InsertAfter(below_box, above_box);
  below_box->AddRef();

  auto above_box_rect = above_box->rect();
  auto below_box_rect = above_box->rect();
  below_box_rect.top = below_box_rect.bottom - new_below_height;
  above_box_rect.bottom = below_box_rect.top - k_cySplitter;
  above_box->SetRect(above_box_rect);
  below_box->Realize(edit_pane_, below_box_rect);
  edit_pane_->SchedulePaintInRect(Rect(rect().left, above_box_rect.bottom,
                                       rect().right, below_box_rect.top));
}

void EditPane::VerticalLayoutBox::StopSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  if (!below_box.GetPrev()) {
    return;
  }

  scoped_refptr<LayoutBox> protect(this);
  auto& above_box = *below_box.GetPrev();
  auto const cyMin = k_cyMinBox;
  if (pt.y - above_box.rect().top < cyMin) {
    below_box.rect().top = above_box.rect().top;
    above_box.Destroy();
    below_box.SetRect(below_box.rect());
    UpdateSplitters();
  } else if (below_box.rect().bottom - pt.y < k_cyMinBox) {
    above_box.rect().bottom = below_box.rect().bottom;
    below_box.Destroy();
    above_box.SetRect(above_box.rect());
    UpdateSplitters();
  }
}

//////////////////////////////////////////////////////////////////////
//
// EditPane::SplitterController
//
class EditPane::SplitterController {
  public: enum State {
    State_None,
    State_Drag,
    State_DragSingle,
  };

  private: const EditPane& owner_;
  private: Box* m_pBox;
  private: State m_eState;

  public: explicit SplitterController(const EditPane&);
  public: ~SplitterController();

  public: bool is_dragging() const { return m_eState != State_None; }
  public: void End(const gfx::Point&);
  public: void Move(const gfx::Point&);
  public: void Start(State, Box&);
  public: void Stop();

  DISALLOW_COPY_AND_ASSIGN(SplitterController);
};

EditPane::SplitterController::SplitterController(const EditPane& owner)
    : owner_(owner),
      m_eState(State_None),
      m_pBox(nullptr) {}

EditPane::SplitterController::~SplitterController() {
  ASSERT(!m_pBox);
}

void EditPane::SplitterController::End(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    UI_DOM_AUTO_LOCK_SCOPE();
    m_pBox->outer()->StopSplitter(point, *m_pBox);
    Stop();
  }
}

void EditPane::SplitterController::Move(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    UI_DOM_AUTO_LOCK_SCOPE();
    m_pBox->outer()->MoveSplitter(point, *m_pBox);
  }
}

void EditPane::SplitterController::Start(State eState, Box& box) {
  ASSERT(!!box.outer());
  const_cast<EditPane&>(owner_).SetCapture();
  m_eState = eState;
  m_pBox = &box;
  box.AddRef();
}

void EditPane::SplitterController::Stop() {
  if (m_eState != State_None) {
    ASSERT(!!m_pBox);
    const_cast<EditPane&>(owner_).ReleaseCapture();
    m_eState = State_None;
    m_pBox->Release();
    m_pBox = nullptr;
  }
  ASSERT(!m_pBox);
}

EditPane::EditPane(Window* pWindow)
    : m_eState(State_NotRealized),
      root_box_(new VerticalLayoutBox(this, nullptr)),
      splitter_controller_(new SplitterController(*this)) {
  AppendChild(pWindow);
  scoped_refptr<LeafBox> box(new LeafBox(this, root_box_, pWindow));
  root_box_->Add(*box);
}

EditPane::~EditPane() {
  root_box_->Removed();
}

Frame& EditPane::frame() const {
  ASSERT(GetFrame());
  return *GetFrame();
}

void EditPane::Activate() {
  Pane::Activate();
  auto const window = GetActiveWindow();
  if (!window)
    return;
  window->RequestFocus();
}

void EditPane::DidRealize() {
  m_eState = State_Realized;
  root_box_->Realize(this, rect());
}

void EditPane::DidRealizeChildWidget(const Widget& window) {
  auto const box = root_box_->FindLeafBoxFromWidget(window);
  if (!box)
    return;

  auto const next_leaf_box = box->GetNext() ?
      box->GetNext()->GetFirstLeafBox() : nullptr;
   auto const next_window = next_leaf_box ? next_leaf_box->GetWindow() :
      nullptr;
  if (next_window)
    InsertBefore(box->GetWindow(), next_window);
  else
    AppendChild(box->GetWindow());
}

void EditPane::DidRemoveChildWidget(const Widget&) {
  if (root_box_->CountLeafBox())
    return;
  // There is no window in this pane. So, we delete this pane.
  DestroyWidget();
}

void EditPane::DidResize() {
  #if DEBUG_RESIZE
    DEBUG_WIDGET_PRINTF(DEBUG_RECT_FORMAT "\n", DEBUG_RECT_ARG(rect()));
  #endif
  root_box_->SetRect(rect());
}

void EditPane::DidSetFocus(ui::Widget*) {
  if (auto const widget = GetActiveWindow())
    widget->RequestFocus();
}

// Returns the last active Box.
EditPane::LeafBox* EditPane::GetActiveLeafBox() const {
  return root_box_->GetActiveLeafBox();
}

// Returns the last active Box.
EditPane::Window* EditPane::GetActiveWindow() const {
  auto const pBox = GetActiveLeafBox();
  return pBox ? pBox->GetWindow() : nullptr;
}

text::Buffer* EditPane::GetBuffer() const {
  auto const window = GetActiveWindow();
  if (!window)
    return nullptr;
  if (auto const text_edit_window = window->as<TextEditWindow>())
    return text_edit_window->buffer();
  return nullptr;
}

HCURSOR EditPane::GetCursorAt(const gfx::Point& point) const {
  auto const result = root_box_->HitTest(point);
  switch (result.type) {
    case HitTestResult::HSplitter:
    case HitTestResult::HSplitterBig: {
      DEFINE_STATIC_LOCAL(StockCursor, hsplit_cursor, (IDC_HSPLIT));
      return hsplit_cursor;
    }

    case HitTestResult::VSplitter:
    case HitTestResult::VSplitterBig: {
      DEFINE_STATIC_LOCAL(StockCursor, vsplit_cursor, (IDC_VSPLIT));
      return vsplit_cursor;
    }

    case HitTestResult::None:
      return nullptr;

    default: {
      DEFINE_STATIC_LOCAL(StockCursor, arrow_cursor, (IDC_ARROW));
      return arrow_cursor;
    }
  }
}

EditPane::Window* EditPane::GetFirstWindow() const {
  auto const window = first_child()->as<Window>();
  if (window)
    return window;
  CAN_NOT_HAPPEN();
}

EditPane::Window* EditPane::GetLastWindow() const {
  auto const window = last_child()->as<Window>();
  if (window)
    return window;
  CAN_NOT_HAPPEN();
}

views::Window* EditPane::GetWindow() const {
  return GetActiveWindow();
}

void EditPane::OnDraw(gfx::Graphics* gfx) {
  root_box_->DrawSplitters(*gfx);
}

void EditPane::OnMouseReleased(const ui::MouseEvent& event) {
  splitter_controller_->End(event.location());
}

void EditPane::OnMouseMoved(const ui::MouseEvent& event) {
  splitter_controller_->Move(event.location());
}

void EditPane::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() && event.click_count() != 1)
    return;
  auto const point = event.location();
  auto const result = root_box_->HitTest(point);
  if (result.type == HitTestResult::HSplitter ||
      result.type == HitTestResult::VSplitter) {
    splitter_controller_->Start(SplitterController::State_Drag,
                                *result.box);
  } else if (result.type == HitTestResult::VSplitterBig) {
    splitter_controller_->Start(SplitterController::State_DragSingle,
                                *result.box);
  }
}

void EditPane::ReplaceActiveWindow(Window* window) {
  DCHECK(!window->is_realized());
  GetActiveLeafBox()->ReplaceWindow(window);
}

void EditPane::SplitHorizontally(Window* left_window,
                                 Window* new_right_window) {
  DCHECK(left_window->is_realized());
  DCHECK_NE(left_window, new_right_window);
  DCHECK(!new_right_window->is_realized());
  auto const left_box = root_box_->FindLeafBoxFromWidget(*left_window);
  DCHECK(left_box);

  auto const width = left_box->rect().width();
  if (width < k_cxMinBox * 2 + k_cxSplitter) {
    frame().AddWindow(new_right_window);
    return;
  }

  left_box->EnsureInHorizontalLayoutBox();
  left_box->outer()->Split(left_box, new_right_window, width / 2);
}

void EditPane::SplitVertically(Window* above_window,
                               Window* new_below_window) {
  DCHECK(above_window->is_realized());
  DCHECK_NE(above_window, new_below_window);
  DCHECK(!new_below_window->is_realized());
  auto const above_box = root_box_->FindLeafBoxFromWidget(*above_window);
  DCHECK(above_box);

  auto const height = above_box->rect().height();
  if (height < k_cyMinBox * 2 + k_cySplitter) {
    frame().AddWindow(new_below_window);
    return;
  }

  above_box->EnsureInVerticalLayoutBox();
  above_box->outer()->Split(above_box, new_below_window, height / 2);
}

void EditPane::WillDestroyWidget() {
  Pane::WillDestroyWidget();
  m_eState = State_Destroyed;
  root_box_->Destroy();
}

void EditPane::WillRemoveChildWidget(const Widget& child) {
  Pane::WillRemoveChildWidget(child);
  auto const box = root_box_->FindLeafBoxFromWidget(child);
  if (!box) {
    // RepalceActiveWindow() removes window from box then destroys window.
    return;
  }

  #if DEBUG_RESIZE
    DEBUG_WIDGET_PRINTF("box=%p\n", box);
  #endif
  box->DetachWindow();
  auto const outer = box->outer();
  outer->RemoveBox(*box);
}
