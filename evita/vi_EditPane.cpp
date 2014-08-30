#include "precomp.h"
// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_REDRAW 0
#define DEBUG_RESIZE 0
#define DEBUG_SPLIT 0
#include "evita/vi_EditPane.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "common/castable.h"
#include "common/tree/child_nodes.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/editor/application.h"
#include "evita/resource.h"
#include "evita/ui/animation/window_animator.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/events/event.h"
#include "evita/views/content_window.h"
#include "evita/views/tab_data_set.h"
#include "evita/vi_Frame.h"
#include "evita/vi_TextEditWindow.h"

extern HINSTANCE g_hInstance;

using namespace views;

namespace {

auto const kMinBoxHeight = 200.0f;
auto const kMinBoxWidth = 200.0f;
auto const kSplitterWidth = 8.0f;
auto const kSplitterHeight = 8.0f;

//////////////////////////////////////////////////////////////////////
//
// Bounds
//
class Bounds {
  private: gfx::RectF bounds_;

  protected: Bounds() = default;
  protected: virtual ~Bounds() = default;

  public: float bottom() const { return bounds_.bottom; }
  public: gfx::PointF bottom_right() const {
    return bounds_.bottom_right();
  }
  public: const gfx::RectF& bounds() const { return bounds_; }
  public: float height() const { return bounds_.height(); }
  public: float left() const { return bounds_.left; }
  public: gfx::PointF origin() const { return bounds_.origin(); }
  public: float right() const { return bounds_.right; }
  public: gfx::SizeF size() const { return bounds_.size(); }
  public: float top() const { return bounds_.top; }
  public: float width() const { return bounds_.width(); }
  public: void set_bounds(const gfx::RectF& new_bounds) {
    bounds_ = new_bounds;
  }

  protected: virtual void DidChangeBounds();
  public: void SetBounds(const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Bounds);
};

void Bounds::DidChangeBounds() {
}

void Bounds::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK_GE(new_bounds.width(), kMinBoxWidth);
  DCHECK_GE(new_bounds.height(), kMinBoxHeight);
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  DidChangeBounds();
}

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
struct HitTestResult {
  enum Type {
    None,
    HScrollBar,
    HSplitter,
    HSplitterBig,
    VScrollBar,
    VSplitter,
    ContentWindow,
  };

  EditPane::Box* box;
  Type type;

  HitTestResult() : box(nullptr), type(None) {
  }

  HitTestResult(Type type, const EditPane::Box* box)
      : box(const_cast<EditPane::Box*>(box)), type(type) {
    DCHECK_NE(type, None);
  }

  explicit operator bool() const { return box; }

  views::ContentWindow* window() const;
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditPane::Box
//
class EditPane::Box : public DoubleLinkedNode_<EditPane::Box>,
                      public Bounds,
                      public base::RefCounted<EditPane::Box>,
                      public common::Castable {
  DECLARE_CASTABLE_CLASS(Box, Castable);

  protected: typedef DoubleLinkedList_<Box> BoxList;

  protected: BoxList boxes_;
  protected: EditPane* edit_pane_;
  private: bool is_removed_;
  private: Box* outer_;

  protected: Box(EditPane* edit_pane, Box* outer);
  public: virtual ~Box();

  public: Box* first_child() const { return boxes_.GetFirst(); }
  public: bool is_removed() const { return is_removed_; }
  public: Box* next_sibling() const { return GetNext(); }
  public: Box* outer() const { return outer_; }
  public: void set_outer(Box* outer) { outer_ = outer; }
  public: Box* previous_sibling() const { return GetPrev(); }

  public: void Add(Box* box);
  public: virtual int CountLeafBox() const;
  public: virtual void Destroy();
  public: virtual void DetachContent();
  public: void DidRemove();
  public: void EnsureInHorizontalBox();
  public: void EnsureInVerticalBox();
  public: virtual Box* FindLeafBoxFromWidget(
      const ui::Widget*) const;
  public: virtual Box* GetActiveLeafBox() const;
  public: virtual views::ContentWindow* GetContent() const;
  public: virtual Box* GetFirstLeafBox() const;
  public: virtual ::HitTestResult HitTest(const gfx::PointF& point) const = 0;
  public: virtual void MoveSplitter(const gfx::PointF&, Box*);
  public: virtual void Realize();
  public: void RemoveBox(Box*);
  public: void Replace(Box*, Box*);
  public: virtual void ReplaceContent(views::ContentWindow* new_content);
  public: virtual void Split(Box* ref_box, ContentWindow* new_window,
                             float new_box_size);
  public: virtual void StopSplitter(const gfx::Point&, Box*);
  public: void WillRemoveChild(Box* child);

  DISALLOW_COPY_AND_ASSIGN(Box);
};

namespace {

//////////////////////////////////////////////////////////////////////
//
// HorizontalBox
//
class HorizontalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(HorizontalBox, Box);

  public: HorizontalBox(EditPane* edit_pane, Box* outer);
  public: virtual ~HorizontalBox() = default;

  // EditPane::Box
  private: virtual void DidChangeBounds() override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void MoveSplitter(const gfx::PointF&, Box*) override;
  private: virtual void Split(Box* ref_box, ContentWindow* new_window,
                              float new_box_size) override;
  private: virtual void StopSplitter(const gfx::Point&, Box*) override;

  DISALLOW_COPY_AND_ASSIGN(HorizontalBox);
};

//////////////////////////////////////////////////////////////////////
//
// LeafBox
//
class LeafBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(LeafBox, Box);

  private: views::ContentWindow* content_;

  public: LeafBox(EditPane* edit_pane, Box* outer,
                  views::ContentWindow* pWindow);
  public: virtual ~LeafBox();

  public: void DetachContent();
  public: views::ContentWindow* GetWindow() const { return content_; }
  private: bool HasSibling() const { return GetNext() || GetPrev(); }

  // Bounds
  private: void DidChangeBounds() override;

  // EditPane::Box
  private: virtual int CountLeafBox() const override;
  private: virtual void Destroy() override;
  private: virtual Box* FindLeafBoxFromWidget(
      const ui::Widget*) const override;
  private: virtual Box* GetActiveLeafBox() const override;
  private: virtual views::ContentWindow* GetContent() const override;
  private: virtual Box* GetFirstLeafBox() const override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Realize() override;
  private: virtual void ReplaceContent(
      views::ContentWindow* new_window) override;

  DISALLOW_COPY_AND_ASSIGN(LeafBox);
};

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
class VerticalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(VerticalBox, Box);

  public: VerticalBox(EditPane* edit_pane, Box* outer);
  public: virtual ~VerticalBox();

  // Bounds
  private: virtual void DidChangeBounds() override;

  // EditPane::Box
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void MoveSplitter(const gfx::PointF&, Box*) override;
  private: virtual void Split(Box* ref_box, ContentWindow* new_window,
                              float new_box_size) override;
  private: virtual void StopSplitter(const gfx::Point&, Box*) override;

  DISALLOW_COPY_AND_ASSIGN(VerticalBox);
};

//////////////////////////////////////////////////////////////////////
//
// StockCursor
//
class StockCursor {
  private: HCURSOR hCursor_;
  private: HINSTANCE hInstance_;
  private: const char16* id_;
  public: explicit StockCursor(int id)
    : StockCursor(g_hInstance, MAKEINTRESOURCE(id)) {
  }
  public: explicit StockCursor(const base::char16* id)
    : StockCursor(nullptr, id) {
  }
  private: StockCursor(HINSTANCE instance, const char16* id)
      : hCursor_(nullptr), hInstance_(instance), id_(id) {
  }
  public: operator HCURSOR() {
    if (!hCursor_) {
      hCursor_ = ::LoadCursor(hInstance_, id_);
      DCHECK(hCursor_);
    }
    return hCursor_;
  }
  DISALLOW_COPY_AND_ASSIGN(StockCursor);
};

//////////////////////////////////////////////////////////////////////
//
// HorizontalBox
//
HorizontalBox::HorizontalBox(EditPane* edit_pane,
                                         Box* outer)
    : Box(edit_pane, outer) {
}

void HorizontalBox::DidChangeBounds() {
  for (;;) {
    auto const num_boxes = boxes_.Count();
    if (!num_boxes)
      return;

    if (num_boxes == 1) {
      boxes_.GetFirst()->SetBounds(bounds());
      return;
    }

    auto old_width = -kSplitterWidth;
    for (const auto& child : boxes_) {
      old_width += child.width() + kSplitterWidth;
    }
    DCHECK_GE(old_width, kMinBoxWidth);
    auto const scale = width() / old_width;

    // Find smallest child box smaller than minimum box.
    auto child_to_remove = static_cast<Box*>(nullptr);
    for (auto& child : boxes_) {
      auto const new_child_width = ::floor(child.width() * scale);
      if (new_child_width < kMinBoxWidth && !child_to_remove &&
          child_to_remove->width() >= child.width()) {
        child_to_remove = &child;
      }
    }

    if (child_to_remove) {
      child_to_remove->Destroy();
      continue;
    }

    // Layout child boxes
    auto num_rest = num_boxes;
    auto child_origin = origin();
    for (auto& child : boxes_){
      --num_rest;
      if (!num_rest) {
        child.SetBounds(gfx::RectF(child_origin, bottom_right()));
        break;
      }
      auto const new_child_width = ::floor(child.width() * scale);
      child.SetBounds(gfx::RectF(
          child_origin, gfx::SizeF(new_child_width, height())));
      child_origin += gfx::SizeF(new_child_width + kSplitterWidth, 0.0f);
    }
    return;
  }
}

HitTestResult HorizontalBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();

  auto num_rest = boxes_.Count();
  for (const auto& child : boxes_) {
    if (auto const result = child.HitTest(point))
      return result;
    --num_rest;
    if (!num_rest)
      break;
    auto const spliiter_bounds = gfx::RectF(
        gfx::PointF(child.right(), child.top()),
        gfx::SizeF(kSplitterWidth, height()));
    if (spliiter_bounds.Contains(point))
      return HitTestResult(HitTestResult::HSplitter, &child);
  }

  NOTREACHED();
  return HitTestResult();
}

void HorizontalBox::MoveSplitter(const gfx::PointF& point,
                                       Box* right_box) {
  if (!right_box->GetPrev())
    return;
  auto& left_box = *right_box->GetPrev();
  if (point.x - left_box.left() <= 0.0f) {
    // Above box is too small.
    return;
  }
  if (right_box->bounds().right - point.x <= kMinBoxWidth) {
    // Below box is too small.
    return;
  }

 left_box.SetBounds(gfx::RectF(
     left_box.bounds().origin(),
     gfx::PointF(point.x, left_box.bounds().bottom)));

 right_box->SetBounds(gfx::RectF(
     gfx::PointF(point.x + kSplitterWidth, right_box->bounds().top),
     right_box->bounds().bottom_right()));
}

void HorizontalBox::Split(Box* left_box,
                          views::ContentWindow* new_right_window,
                          float new_right_width) {
  DCHECK(!new_right_window->is_realized());
  edit_pane_->AppendChild(new_right_window);

  auto right_box = new LeafBox(edit_pane_, this, new_right_window);
  boxes_.InsertAfter(right_box, left_box);
  right_box->AddRef();

  left_box->set_bounds(gfx::RectF(left_box->origin(),
      left_box->size() - gfx::SizeF(new_right_width + kSplitterWidth, 0.0f)));
  edit_pane_->window_animator()->SlideInFromRight(
      left_box->GetContent(), left_box->width(), new_right_window,
      kSplitterWidth);
  right_box->SetBounds(gfx::RectF(
      gfx::PointF(left_box->right() + kSplitterWidth, top()),
      gfx::SizeF(new_right_width, height())));
  new_right_window->RealizeWidget();
}

void HorizontalBox::StopSplitter(const gfx::Point& point,
                                       Box* below_box) {
  auto const above_box = below_box->GetPrev();
  if (!above_box)
    return;

  // This box may be destroyed when it has only one child which is layout box.
  scoped_refptr<Box> protect(this);

  if (point.x() - above_box->left() < kMinBoxWidth) {
    auto const above_box_origin = above_box->origin();
    above_box->Destroy();
    below_box->SetBounds(gfx::RectF(above_box_origin,
                                    below_box->bottom_right()));
  } else if (below_box->bounds().right - point.x() < kMinBoxWidth) {
    auto const below_box_bottom_right  = below_box->bottom_right();
    below_box->Destroy();
    above_box->SetBounds(gfx::RectF(above_box->origin(),
                                    below_box_bottom_right));
  }
}

//////////////////////////////////////////////////////////////////////
//
// HitTestResult
//
views::ContentWindow* HitTestResult::window() const {
  return box->GetContent();
}

//////////////////////////////////////////////////////////////////////
//
// LeafBox
//
LeafBox::LeafBox(EditPane* edit_pane, Box* outer,
                 views::ContentWindow* content)
    : Box(edit_pane, outer), content_(content) {
}

LeafBox::~LeafBox() {
  DCHECK(!content_);
}

void LeafBox::DetachContent() {
  content_->RemoveObserver(edit_pane_);
  content_ = nullptr;
}

// Bounds
void LeafBox::DidChangeBounds() {
  Box::DidChangeBounds();
  content_->SetBounds(gfx::ToEnclosingRect(bounds()));
}

// EditPane::Box
int LeafBox::CountLeafBox() const {
  return 1;
}

void LeafBox::Destroy() {
  GetWindow()->DestroyWidget();
}

EditPane::Box* LeafBox::GetActiveLeafBox() const {
  return const_cast<LeafBox*>(this);
}

views::ContentWindow* LeafBox::GetContent() const {
  return content_;
}

EditPane::Box* LeafBox::GetFirstLeafBox() const {
  return const_cast<LeafBox*>(this);
}

EditPane::Box* LeafBox::FindLeafBoxFromWidget(const ui::Widget* window) const {
  return window == content_ ? const_cast<LeafBox*>(this) : nullptr;
}

HitTestResult LeafBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(HitTestResult::ContentWindow, this);
}

void LeafBox::Realize() {
  Box::Realize();
  edit_pane_->window_animator()->Realize(content_);
  content_->Realize(gfx::ToEnclosingRect(bounds()));
}

void LeafBox::ReplaceContent(views::ContentWindow* content) {
  DCHECK(!content->parent_node());
  DCHECK(!content->is_realized());
  auto const previous_content = content_;
  content_ = content;
  edit_pane_->AppendChild(content);
  edit_pane_->window_animator()->Replace(content_, previous_content);
  content_->Realize(gfx::ToEnclosingRect(bounds()));
}

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
VerticalBox::VerticalBox(EditPane* edit_pane, Box* outer)
    : Box(edit_pane, outer) {}

VerticalBox::~VerticalBox() {
}

// EditPane::Box
void VerticalBox::DidChangeBounds() {
  for (;;) {
    auto const num_boxes = boxes_.Count();
    if (!num_boxes)
      return;

    if (num_boxes == 1) {
      boxes_.GetFirst()->SetBounds(bounds());
      return;
    }

    auto old_height = -kSplitterHeight;
    for (const auto& child : boxes_) {
      old_height += child.height() + kSplitterHeight;
    }
    DCHECK_GE(old_height, kMinBoxHeight);
    auto const scale = height() / old_height;

    // Find smallest child box smaller than minimum box.
    auto child_to_remove = static_cast<Box*>(nullptr);
    for (auto& child : boxes_) {
      auto const new_child_height = ::floor(child.height() * scale);
      if (new_child_height < kMinBoxHeight && !child_to_remove &&
          child_to_remove->height() >= child.height()) {
        child_to_remove = &child;
      }
    }

    if (child_to_remove) {
      child_to_remove->Destroy();
      continue;
    }

    // Layout child boxes
    auto num_rest = num_boxes;
    auto child_origin = origin();
    for (auto& child : boxes_){
      --num_rest;
      if (!num_rest) {
        child.SetBounds(gfx::RectF(child_origin, bottom_right()));
        break;
      }
      auto const new_child_height = ::floor(child.height() * scale);
      child.SetBounds(gfx::RectF(
          child_origin, gfx::SizeF(width(), new_child_height)));
      child_origin += gfx::SizeF(0.0f, new_child_height + kSplitterHeight);
    }
    return;
  }
}

HitTestResult VerticalBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();

  for (const auto& child : boxes_) {
    if (auto const result = child.HitTest(point))
      return result;

    auto const splitter_bounds = gfx::RectF(
        gfx::PointF(left(), child.bottom()),
        gfx::SizeF(width(), kSplitterHeight));
    if (splitter_bounds.Contains(point))
      return HitTestResult(HitTestResult::VSplitter, &child);
  }

  return HitTestResult();
}

void VerticalBox::MoveSplitter(const gfx::PointF& point,
                                     Box* below_box) {
  auto const above_box = below_box->GetPrev();
  if (!above_box)
    return;

  if (point.y - above_box->top() <= 0) {
    // Above box is too small.
    return;
  }

  if (below_box->bottom() - (point.y + kSplitterHeight) <= 0) {
    // Below box is too small.
    return;
  }

  above_box->SetBounds(gfx::RectF(
      above_box->origin(),
      gfx::PointF(above_box->right(), point.y)));
  below_box->SetBounds(gfx::RectF(
      gfx::PointF(below_box->left(), point.y + kSplitterHeight),
      below_box->bottom_right()));
}

void VerticalBox::Split(Box* above_box,
                        views::ContentWindow* below_window,
                        float below_box_height) {
  DCHECK(!below_window->is_realized());
  DCHECK_GE(below_box_height, kMinBoxHeight);

  auto const above_box_height = above_box->height() - below_box_height -
                                kSplitterHeight;
  DCHECK_GE(above_box_height, kMinBoxHeight);

  auto const below_box = new LeafBox(edit_pane_, this, below_window);
  boxes_.InsertAfter(below_box, above_box);
  below_box->AddRef();

  above_box->set_bounds(gfx::RectF(
      above_box->origin(), gfx::SizeF(width(), above_box_height)));
  edit_pane_->window_animator()->SlideInFromBottom(
    above_box->GetContent(), above_box_height, below_window, kSplitterHeight);
  below_box->SetBounds(gfx::RectF(
      gfx::PointF(left(), above_box->bottom() + kSplitterHeight),
      gfx::SizeF(width(), below_box_height)));
  edit_pane_->AppendChild(below_window);
  below_window->RealizeWidget();
}

void VerticalBox::StopSplitter(const gfx::Point& point,
                                     Box* below_box) {
  auto const above_box = below_box->GetPrev();
  if (!above_box)
    return;

  // This box may be destroyed when it has only one child which is layout box.
  scoped_refptr<Box> protect(this);

  if (point.y() - above_box->top() < kMinBoxHeight) {
    below_box->SetBounds(gfx::RectF(above_box->origin(),
                                    below_box->bottom_right()));
    above_box->Destroy();
  } else if (below_box->bottom() - point.y() < kMinBoxHeight) {
    above_box->SetBounds(gfx::RectF(above_box->origin(),
                                    below_box->bottom_right()));
    below_box->Destroy();
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditPane::Box
//
EditPane::Box::Box(EditPane* edit_pane, Box* outer)
    : edit_pane_(edit_pane),
      is_removed_(false),
      outer_(outer) {
}

EditPane::Box::~Box() {
  DCHECK(boxes_.IsEmpty());
  DCHECK(is_removed_);
  DCHECK(!outer_);
  DCHECK(!GetNext());
  DCHECK(!GetPrev());
}

void EditPane::Box::Add(Box* box) {
  DCHECK(!is_removed());
  boxes_.Append(box);
  box->AddRef();
}

int EditPane::Box::CountLeafBox() const {
  DCHECK(!is_removed());
  auto count = 0;
  for (const auto& child : boxes_) {
    count += child.CountLeafBox();
  }
  return count;
}

void EditPane::Box::Destroy() {
  DCHECK(!is_removed());
  scoped_refptr<Box> protect(this);
  while (auto const box = boxes_.GetFirst()) {
    box->Destroy();
  }
}

void EditPane::Box::DetachContent() {
  NOTREACHED();
}

void EditPane::Box::DidRemove() {
  DCHECK(!is_removed());
  is_removed_ = true;
  outer_ = nullptr;
}

void EditPane::Box::EnsureInHorizontalBox() {
  if (!outer()->is<VerticalBox>())
    return;

  auto const layout_box = new HorizontalBox(edit_pane_, outer());
  scoped_refptr<Box> protect(this);
  outer()->Replace(layout_box, this);
  layout_box->SetBounds(bounds());
  layout_box->Realize();
  layout_box->Add(this);
  set_outer(layout_box);
}

void EditPane::Box::EnsureInVerticalBox() {
  if (outer()->is<VerticalBox>())
    return;

  auto const layout_box = new VerticalBox(edit_pane_, outer());
  scoped_refptr<Box> protect(this);
  outer()->Replace(layout_box, this);
  layout_box->SetBounds(bounds());
  layout_box->Realize();
  layout_box->Add(this);
  set_outer(layout_box);
}

EditPane::Box* EditPane::Box::FindLeafBoxFromWidget(
    const ui::Widget* window) const {
  DCHECK(!is_removed());
  for (const auto& child : boxes_) {
    if (auto const box = child.FindLeafBoxFromWidget(window))
      return box;
  }
  return nullptr;
}

EditPane::Box* EditPane::Box::GetActiveLeafBox() const {
  DCHECK(!is_removed());
  class Local {
    private: static int ActiveTick(const Box* box) {
      auto const content = box->GetContent();
      return content->visible() ? content->active_tick() : 0;
    }

    public: static Box* SelectActiveBox(Box* box1, Box* box2) {
      return box1 && box2 ? ActiveTick(box1) > ActiveTick(box2) ?
          box1 : box2 : box1 ? box1 : box2;
    }
  };

  auto candidate = static_cast<Box*>(nullptr);
  for (auto& box : boxes_) {
    auto const other = box.GetActiveLeafBox();
    //if (other && other->GetWindow()->visible())
    candidate = Local::SelectActiveBox(candidate, other);
  }
  return candidate;
}

views::ContentWindow* EditPane::Box::GetContent() const {
  NOTREACHED();
  return nullptr;
}

EditPane::Box* EditPane::Box::GetFirstLeafBox() const {
  DCHECK(!is_removed());
  return boxes_.GetFirst() ? boxes_.GetFirst()->GetFirstLeafBox() : nullptr;
}

void EditPane::Box::MoveSplitter(const gfx::PointF&, Box*) {
  NOTREACHED();
}

void EditPane::Box::Realize() {
  DCHECK(!is_removed());
  DCHECK(!bounds().empty());
  for (auto& child : boxes_) {
    child.Realize();
  }
}

void EditPane::Box::RemoveBox(Box* box) {
  DCHECK(!is_removed());

  scoped_refptr<Box> protect(this);

  WillRemoveChild(box);
  boxes_.Delete(box);
  box->DidRemove();
  box->Release();

  if (!outer())
    return;

  if (boxes_.IsEmpty()) {
    outer()->RemoveBox(this);
    return;
  }

  if (boxes_.GetFirst() != boxes_.GetLast())
    return;

  // Move only child to outer box.
  auto const first_box = boxes_.GetFirst();
  boxes_.Delete(first_box);
  auto const outer = this->outer();
  DidRemove();
  outer->Replace(first_box, this);
  first_box->Release();
  return;
}

void EditPane::Box::Replace(Box* new_box, Box* old_box) {
  DCHECK(!is_removed());
  boxes_.InsertBefore(new_box, old_box);
  new_box->set_outer(this);
  new_box->AddRef();
  boxes_.Delete(old_box);
  old_box->Release();
}

void EditPane::Box::ReplaceContent(views::ContentWindow*) {
  NOTREACHED();
}

void EditPane::Box::Split(Box*, views::ContentWindow*, float) {
  NOTREACHED();
}

void EditPane::Box::StopSplitter(const gfx::Point&, Box*) {
  NOTREACHED();
}

void EditPane::Box::WillRemoveChild(Box* child) {
  if (auto const previous_sibling = child->GetPrev()) {
    previous_sibling->SetBounds(gfx::RectF(
        previous_sibling->origin(), child->bottom_right()));
  } else if (auto const next_sibling = child->GetNext()) {
    next_sibling->SetBounds(gfx::RectF(
        child->origin(), next_sibling->bottom_right()));
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
  DCHECK(!m_pBox);
}

void EditPane::SplitterController::End(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    m_pBox->outer()->StopSplitter(point, m_pBox);
    Stop();
  }
}

void EditPane::SplitterController::Move(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    m_pBox->outer()->MoveSplitter(gfx::PointF(point), m_pBox);
  }
}

void EditPane::SplitterController::Start(State eState, Box& box) {
  DCHECK(!!box.outer());
  const_cast<EditPane&>(owner_).SetCapture();
  m_eState = eState;
  m_pBox = &box;
  box.AddRef();
}

void EditPane::SplitterController::Stop() {
  if (m_eState != State_None) {
    DCHECK(!!m_pBox);
    const_cast<EditPane&>(owner_).ReleaseCapture();
    m_eState = State_None;
    m_pBox->Release();
    m_pBox = nullptr;
  }
  DCHECK(!m_pBox);
}

//////////////////////////////////////////////////////////////////////
//
// EditPane
//
EditPane::EditPane()
    : window_animator_(new ui::WindowAnimator()),
      root_box_(new VerticalBox(this, nullptr)),
      splitter_controller_(new SplitterController(*this)) {
}

EditPane::~EditPane() {
  root_box_->DidRemove();
}

ui::WindowAnimator* EditPane::window_animator() const {
  return window_animator_.get();
}

bool EditPane::has_more_than_one_child() const {
  return first_child() != last_child();
}

// Returns the last active Box.
views::ContentWindow* EditPane::GetActiveWindow() const {
  auto const box = root_box_->GetActiveLeafBox();
  return box ? box->GetContent() : nullptr;
}

void EditPane::ReplaceActiveContent(views::ContentWindow* content) {
  DCHECK(!content->is_realized());
  root_box_->GetActiveLeafBox()->ReplaceContent(content);
}

void EditPane::SetContent(views::ContentWindow* content) {
  DCHECK(!root_box_->first_child());
  scoped_refptr<LeafBox> box(new LeafBox(this, root_box_.get(), content));
  root_box_->Add(box);
  if (bounds().empty()) {
    DCHECK(!is_realized());
  } else {
    auto const box_bounds = GetContentsBounds();
    box->SetBounds(box_bounds);
    content->SetBounds(gfx::ToEnclosingRect(box_bounds));
  }

  if (is_realized())
    content->AddObserver(this);
  if (!content->is_realized())
    window_animator()->Realize(content);
  content->SetParentWidget(this);
}

void EditPane::SplitHorizontally(ContentWindow* left_window,
                                 ContentWindow* new_right_window) {
  DCHECK(left_window->is_realized());
  DCHECK_NE(left_window, new_right_window);
  DCHECK(!new_right_window->is_realized());
  auto const left_box = root_box_->FindLeafBoxFromWidget(left_window);
  DCHECK(left_box);

  auto const left_box_width = ::floor(
    (left_box->bounds().width() - kSplitterWidth) / 2);
  auto const right_box_width = left_box->width() - left_box_width;
  if (left_box_width < kMinBoxWidth || right_box_width < kMinBoxWidth) {
    GetFrame()->AddOrActivateTabContent(new_right_window);
    return;
  }

  left_box->EnsureInHorizontalBox();
  left_box->outer()->Split(left_box, new_right_window, right_box_width);
  left_window->MakeSelectionVisible();
}

void EditPane::SplitVertically(ContentWindow* above_window,
                               ContentWindow* new_below_window) {
  DCHECK(above_window->is_realized());
  DCHECK_NE(above_window, new_below_window);
  DCHECK(!new_below_window->is_realized());
  auto const above_box = root_box_->FindLeafBoxFromWidget(above_window);
  DCHECK(above_box);

  auto const above_box_height = ::floor(
    (above_box->bounds().height() - kSplitterHeight) / 2);
  auto const below_box_height = above_box->height() - above_box_height;
  if (above_box_height < kMinBoxHeight || below_box_height < kMinBoxHeight) {
    GetFrame()->AddOrActivateTabContent(new_below_window);
    return;
  }

  above_box->EnsureInVerticalBox();
  above_box->outer()->Split(above_box, new_below_window, below_box_height);
  above_window->MakeSelectionVisible();
}

// ui::AnimationObserver
void EditPane::DidAnimate(ui::Animatable* animatable) {
  animated_contents_.insert(animatable);
  animatable->RemoveObserver(this);
  for (auto child : child_nodes()) {
    auto const animatable = child->as<ui::AnimatableWindow>();
    if (!animatable)
      continue;
    if (animated_contents_.find(animatable) == animated_contents_.end())
      return;
  }
  // All contents are animated, we notify observers this tab content is
  // animated.
  DidAnimateTabContent();
}

// ui::Widget
void EditPane::DidChangeBounds() {
  TabContent::DidChangeBounds();
  root_box_->SetBounds(GetContentsBounds());
  animated_contents_.clear();
}

void EditPane::DidHide() {
  TabContent::DidHide();
  animated_contents_.clear();
  for (auto child : child_nodes()) {
    auto const animatable = child->as<ui::AnimatableWindow>();
    if (!animatable)
      continue;
    animatable->RemoveObserver(this);
  }
}

void EditPane::DidRealizeChildWidget(const ui::Widget& window) {
  TabContent::DidRealizeChildWidget(window);
  auto const box = root_box_->FindLeafBoxFromWidget(&window);
  if (!box)
    return;

  auto const next_leaf_box = box->GetNext() ?
      box->GetNext()->GetFirstLeafBox() : nullptr;
   auto const next_window = next_leaf_box ? next_leaf_box->GetContent() :
      nullptr;
  if (next_window)
    InsertBefore(box->GetContent(), next_window);
  else
    AppendChild(box->GetContent());
}

void EditPane::DidRemoveChildWidget(const Widget&) {
  if (root_box_->CountLeafBox())
    return;
  // There is no window in this pane. So, we delete this pane.
  DestroyWidget();
}

void EditPane::DidSetFocus(ui::Widget*) {
  auto const widget = GetActiveWindow();
  if (!widget)
    return;
  widget->RequestFocus();
}

void EditPane::DidShow() {
  TabContent::DidShow();
  animated_contents_.clear();
  for (auto child : child_nodes()) {
    auto const animatable = child->as<ui::AnimatableWindow>();
    if (!animatable)
      continue;
    animatable->AddObserver(this);
  }
}

HCURSOR EditPane::GetCursorAt(const gfx::Point& point) const {
  auto const result = root_box_->HitTest(gfx::PointF(point));
  switch (result.type) {
    case ::HitTestResult::HSplitter:
    case ::HitTestResult::HSplitterBig: {
      DEFINE_STATIC_LOCAL(StockCursor, hsplit_cursor, (IDC_HSPLIT));
      return hsplit_cursor;
    }

    case ::HitTestResult::VSplitter: {
      DEFINE_STATIC_LOCAL(StockCursor, vsplit_cursor, (IDC_VSPLIT));
      return vsplit_cursor;
    }

    case ::HitTestResult::None:
      return nullptr;

    default: {
      DEFINE_STATIC_LOCAL(StockCursor, arrow_cursor, (IDC_ARROW));
      return arrow_cursor;
    }
  }
}

void EditPane::OnMouseReleased(const ui::MouseEvent& event) {
  splitter_controller_->End(event.location());
}

void EditPane::OnMouseMoved(const ui::MouseEvent& event) {
  splitter_controller_->Move(event.location());
}

void EditPane::OnMousePressed(const ui::MouseEvent& event) {
  // We start splitter dragging when left button pressed.
  if (!event.is_left_button() || event.click_count())
    return;
  auto const point = gfx::PointF(event.location());
  auto const result = root_box_->HitTest(point);
  if (result.type == ::HitTestResult::HSplitter ||
      result.type == ::HitTestResult::VSplitter) {
    splitter_controller_->Start(SplitterController::State_Drag,
                                *result.box);
  }
}

void EditPane::WillDestroyWidget() {
  TabContent::WillDestroyWidget();
  root_box_->Destroy();
}

void EditPane::WillRemoveChildWidget(const Widget& child) {
  TabContent::WillRemoveChildWidget(child);
  auto const box = root_box_->FindLeafBoxFromWidget(&child);
  if (!box) {
    // RepalceActiveWindow() removes window from box then destroys window.
    return;
  }

  box->DetachContent();
  box->outer()->RemoveBox(box);
}

// views::TabContent
const domapi::TabData* EditPane::GetTabData() const {
  auto const content = GetActiveWindow();
  if (!content)
    return nullptr;
  return views::TabDataSet::instance()->GetTabData(content->window_id());
}
