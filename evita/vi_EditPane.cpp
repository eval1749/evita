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
#include "common/tree/descendants_or_self.h"
#include "common/tree/node.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/editor/application.h"
#include "evita/resource.h"
#include "evita/ui/animation/animation_observer.h"
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
    Content,
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

  ContentWindow* window() const;
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditPane::Box
//
class EditPane::Box : public Bounds,
                      public base::RefCounted<EditPane::Box>,
                      public common::tree::Node<EditPane::Box>,
                      public common::Castable {
  DECLARE_CASTABLE_CLASS(Box, Castable);

  protected: EditPane* edit_pane_;
  private: bool is_content_visible_;
  private: bool is_removed_;

  protected: Box(EditPane* edit_pane);
  public: virtual ~Box();

  public: bool is_removed() const { return is_removed_; }

  public: int CountLeafBox() const;
  public: virtual void Destroy();
  public: virtual void DetachContent();
  public: virtual void DidHide();
  public: void DidRemove();
  public: virtual void DidShow();
  public: void DidShowChildContent(Box* child);
  public: void EnsureInHorizontalBox();
  public: void EnsureInVerticalBox();
  public: Box* FindLeafBoxFromWidget(const ui::Widget*) const;
  public: Box* GetActiveLeafBox() const;
  public: virtual ContentWindow* GetContent() const;
  public: Box* GetFirstLeafBox() const;
  public: virtual ::HitTestResult HitTest(const gfx::PointF& point) const = 0;
  public: virtual void MoveSplitter(const gfx::PointF&, Box*);
  public: virtual void Realize();
  public: void RemoveBox(Box*);
  public: virtual void ReplaceContent(ContentWindow* new_content);
  public: virtual void Split(Box* ref_box, ContentWindow* new_window,
                             float new_box_size);
  public: virtual void StopSplitter(const gfx::Point&, Box*);
  public: void WillRemoveChild(Box* child);

  // Bounds
  protected: void DidChangeBounds() override;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

namespace {

//////////////////////////////////////////////////////////////////////
//
// HorizontalBox
//
class HorizontalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(HorizontalBox, Box);

  public: HorizontalBox(EditPane* edit_pane);
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
class LeafBox final : public EditPane::Box, public ui::AnimationObserver {
  DECLARE_CASTABLE_CLASS(LeafBox, Box);

  private: ContentWindow* content_;
  private: bool observing_;

  public: LeafBox(EditPane* edit_pane, ContentWindow* content);
  public: virtual ~LeafBox();

  public: void DetachContent();
  private: void DoNotObserveContent();
  private: void ObserveContent();

  // Bounds
  private: void DidChangeBounds() override;

  // EditPane::Box
  private: virtual void Destroy() override;
  private: virtual void DidHide() override;
  private: virtual void DidShow() override;
  private: virtual ContentWindow* GetContent() const override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Realize() override;
  private: virtual void ReplaceContent(ContentWindow* new_window) override;

  // ui::AnimationObserver
  private: virtual void DidAnimate(ui::Animatable* animatable) override;

  DISALLOW_COPY_AND_ASSIGN(LeafBox);
};

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
class VerticalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(VerticalBox, Box);

  public: VerticalBox(EditPane* edit_pane);
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
HorizontalBox::HorizontalBox(EditPane* edit_pane)
    : Box(edit_pane) {
}

void HorizontalBox::DidChangeBounds() {
  for (;;) {
    auto num_boxes = 0;
    for (auto child : child_nodes()) {
      static_cast<void>(child);
      ++num_boxes;
    }
    if (!num_boxes)
      return;

    if (num_boxes == 1) {
      first_child()->SetBounds(bounds());
      return;
    }

    auto old_width = -kSplitterWidth;
    for (const auto child : child_nodes()) {
      old_width += child->width() + kSplitterWidth;
    }
    DCHECK_GE(old_width, kMinBoxWidth);
    auto const scale = width() / old_width;

    // Find smallest child box smaller than minimum box.
    auto child_to_remove = static_cast<Box*>(nullptr);
    for (auto child : child_nodes()) {
      auto const new_child_width = ::floor(child->width() * scale);
      if (new_child_width < kMinBoxWidth && !child_to_remove &&
          child_to_remove->width() >= child->width()) {
        child_to_remove = child;
      }
    }

    if (child_to_remove) {
      child_to_remove->Destroy();
      continue;
    }

    // Layout child boxes
    auto num_rest = num_boxes;
    auto child_origin = origin();
    for (auto child : child_nodes()){
      --num_rest;
      if (!num_rest) {
        child->SetBounds(gfx::RectF(child_origin, bottom_right()));
        break;
      }
      auto const new_child_width = ::floor(child->width() * scale);
      child->SetBounds(gfx::RectF(
          child_origin, gfx::SizeF(new_child_width, height())));
      child_origin += gfx::SizeF(new_child_width + kSplitterWidth, 0.0f);
    }
    return;
  }
}

HitTestResult HorizontalBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();

  for (const auto child : child_nodes()) {
    if (auto const result = child->HitTest(point))
      return result;
    if (!child->next_sibling())
      break;
    auto const spliiter_bounds = gfx::RectF(
        gfx::PointF(child->right(), child->top()),
        gfx::SizeF(kSplitterWidth, height()));
    if (spliiter_bounds.Contains(point))
      return HitTestResult(HitTestResult::HSplitter, child);
  }

  NOTREACHED();
  return HitTestResult();
}

void HorizontalBox::MoveSplitter(const gfx::PointF& point, Box* right_box) {
  if (!right_box->previous_sibling())
    return;
  auto& left_box = *right_box->previous_sibling();
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
                          ContentWindow* new_right_window,
                          float new_right_width) {
  DCHECK(!new_right_window->is_realized());
  edit_pane_->AppendChild(new_right_window);

  auto right_box = new LeafBox(edit_pane_, new_right_window);
  InsertAfter(right_box, left_box);
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

void HorizontalBox::StopSplitter(const gfx::Point& point, Box* below_box) {
  auto const above_box = below_box->previous_sibling();
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
ContentWindow* HitTestResult::window() const {
  return box->GetContent();
}

//////////////////////////////////////////////////////////////////////
//
// LeafBox
//
LeafBox::LeafBox(EditPane* edit_pane, ContentWindow* content)
    : Box(edit_pane), content_(content), observing_(false) {
  if (!content_->is_realized())
    return;
  ObserveContent();
}

LeafBox::~LeafBox() {
  DCHECK(!content_);
  DCHECK(!observing_);
}

void LeafBox::DetachContent() {
  DoNotObserveContent();
  content_ = nullptr;
}

void LeafBox::DoNotObserveContent() {
  if (!observing_)
    return;
  observing_ = false;
  content_->RemoveObserver(this);
}

void LeafBox::ObserveContent() {
  if (observing_)
    return;
  observing_ = true;
  content_->AddObserver(this);
}

// Bounds
void LeafBox::DidChangeBounds() {
  Box::DidChangeBounds();
  ObserveContent();
  content_->SetBounds(gfx::ToEnclosingRect(bounds()));
}

// EditPane::Box
void LeafBox::Destroy() {
  DoNotObserveContent();
  content_->DestroyWidget();
}

void LeafBox::DidHide() {
  Box::DidHide();
  DoNotObserveContent();
}

void LeafBox::DidShow() {
  Box::DidShow();
  ObserveContent();
}

ContentWindow* LeafBox::GetContent() const {
  return content_;
}

HitTestResult LeafBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(HitTestResult::Content, this);
}

void LeafBox::Realize() {
  Box::Realize();
  ObserveContent();
  edit_pane_->window_animator()->Realize(content_);
  content_->Realize(gfx::ToEnclosingRect(bounds()));
}

void LeafBox::ReplaceContent(ContentWindow* content) {
  DCHECK(!content->parent_node());
  DCHECK(!content->is_realized());
  auto const previous_content = content_;
  content_ = content;
  edit_pane_->AppendChild(content);
  edit_pane_->window_animator()->Replace(content_, previous_content);
  content_->Realize(gfx::ToEnclosingRect(bounds()));
}

// ui::AnimationObserver
void LeafBox::DidAnimate(ui::Animatable* animatable) {
  DCHECK_EQ(animatable, content_);
  DoNotObserveContent();
  parent_node()->DidShowChildContent(this);
}

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
VerticalBox::VerticalBox(EditPane* edit_pane)
    : Box(edit_pane) {}

VerticalBox::~VerticalBox() {
}

// EditPane::Box
void VerticalBox::DidChangeBounds() {
  for (;;) {
    if (!first_child())
      return;
    if (!first_child()->next_sibling()) {
      first_child()->SetBounds(bounds());
      return;
    }

    auto old_height = -kSplitterHeight;
    for (const auto child : child_nodes()) {
      old_height += child->height() + kSplitterHeight;
    }
    DCHECK_GE(old_height, kMinBoxHeight);
    auto const scale = height() / old_height;

    // Find smallest child box smaller than minimum box.
    auto child_to_remove = static_cast<Box*>(nullptr);
    for (auto child : child_nodes()) {
      auto const new_child_height = ::floor(child->height() * scale);
      if (new_child_height < kMinBoxHeight && !child_to_remove &&
          child_to_remove->height() >= child->height()) {
        child_to_remove = child;
      }
    }

    if (child_to_remove) {
      child_to_remove->Destroy();
      continue;
    }

    // Layout child boxes
    auto child_origin = origin();
    for (auto child : child_nodes()){
      if (!child->next_sibling()) {
        child->SetBounds(gfx::RectF(child_origin, bottom_right()));
        break;
      }
      auto const new_child_height = ::floor(child->height() * scale);
      child->SetBounds(gfx::RectF(
          child_origin, gfx::SizeF(width(), new_child_height)));
      child_origin += gfx::SizeF(0.0f, new_child_height + kSplitterHeight);
    }
    return;
  }
}

HitTestResult VerticalBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();

  for (const auto child : child_nodes()) {
    if (auto const result = child->HitTest(point))
      return result;

    auto const splitter_bounds = gfx::RectF(
        gfx::PointF(left(), child->bottom()),
        gfx::SizeF(width(), kSplitterHeight));
    if (splitter_bounds.Contains(point))
      return HitTestResult(HitTestResult::VSplitter, child);
  }

  return HitTestResult();
}

void VerticalBox::MoveSplitter(const gfx::PointF& point, Box* below_box) {
  auto const above_box = below_box->previous_sibling();
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
                        ContentWindow* below_window,
                        float below_box_height) {
  DCHECK(!below_window->is_realized());
  DCHECK_GE(below_box_height, kMinBoxHeight);

  auto const above_box_height = above_box->height() - below_box_height -
                                kSplitterHeight;
  DCHECK_GE(above_box_height, kMinBoxHeight);

  auto const below_box = new LeafBox(edit_pane_, below_window);
  InsertAfter(below_box, above_box);
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

void VerticalBox::StopSplitter(const gfx::Point& point, Box* below_box) {
  auto const above_box = below_box->previous_sibling();
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
EditPane::Box::Box(EditPane* edit_pane)
    : edit_pane_(edit_pane), is_content_visible_(false), is_removed_(false) {
}

EditPane::Box::~Box() {
  DCHECK(!first_child());
  DCHECK(is_removed_);
  DCHECK(!parent_node());
  DCHECK(!next_sibling());
  DCHECK(!previous_sibling());
}

int EditPane::Box::CountLeafBox() const {
  DCHECK(!is_removed());
  auto count = 0;
  for (const auto child : common::tree::descendants_or_self(this)) {
    if (child->is<LeafBox>())
      ++count;
  }
  return count;
}

void EditPane::Box::Destroy() {
  DCHECK(!is_removed());
  scoped_refptr<Box> protect(this);
  while (auto const box = first_child()) {
    box->Destroy();
  }
}

void EditPane::Box::DetachContent() {
  NOTREACHED();
}

void EditPane::Box::DidHide() {
  is_content_visible_ = false;
  for (auto const child : child_nodes()) {
    child->DidHide();
  }
}

void EditPane::Box::DidRemove() {
  DCHECK(!is_removed());
  is_removed_ = true;
}

void EditPane::Box::DidShow() {
  is_content_visible_ = false;
  for (auto const child : child_nodes()) {
    child->DidShow();
  }
}

void EditPane::Box::DidShowChildContent(Box* child) {
  child->is_content_visible_ = true;
  for (auto const child : child_nodes()) {
    if (!child->is_content_visible_)
      return;
  }
  if (parent_node()) {
    parent_node()->DidShowChildContent(this);
    return;
  }
  // All contents are animated, we notify observers this tab content is
  // animated.
  edit_pane_->DidAnimateTabContent();
}

void EditPane::Box::EnsureInHorizontalBox() {
  if (!parent_node()->is<VerticalBox>())
    return;

  auto const layout_box = new HorizontalBox(edit_pane_);
  parent_node()->ReplaceChild(layout_box, this);
  layout_box->SetBounds(bounds());
  layout_box->Realize();
  layout_box->AppendChild(this);
}

void EditPane::Box::EnsureInVerticalBox() {
  if (parent_node()->is<VerticalBox>())
    return;

  auto const layout_box = new VerticalBox(edit_pane_);
  parent_node()->ReplaceChild(layout_box, this);
  layout_box->SetBounds(bounds());
  layout_box->Realize();
  layout_box->AppendChild(this);
}

EditPane::Box* EditPane::Box::FindLeafBoxFromWidget(
    const ui::Widget* window) const {
  DCHECK(!is_removed());
  for (const auto child : common::tree::descendants_or_self(this)) {
    if (child->GetContent() == window)
      return const_cast<Box*>(child);
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
  for (auto child : common::tree::descendants_or_self(this)) {
    auto const leaf_box = const_cast<LeafBox*>(child->as<LeafBox>());
    if (!leaf_box)
      continue;
    candidate = Local::SelectActiveBox(candidate, leaf_box);
  }
  return candidate;
}

ContentWindow* EditPane::Box::GetContent() const {
  return nullptr;
}

EditPane::Box* EditPane::Box::GetFirstLeafBox() const {
  DCHECK(!is_removed());
  for (auto child : common::tree::descendants_or_self(this)) {
    if (auto const leaf_box = const_cast<LeafBox*>(child->as<LeafBox>()))
      return leaf_box;
  }
  return nullptr;
}

void EditPane::Box::MoveSplitter(const gfx::PointF&, Box*) {
  NOTREACHED();
}

void EditPane::Box::Realize() {
  DCHECK(!is_removed());
  DCHECK(!bounds().empty());
  for (auto child : child_nodes()) {
    child->Realize();
  }
}

void EditPane::Box::RemoveBox(Box* box) {
  DCHECK(!is_removed());

  scoped_refptr<Box> protect(this);

  WillRemoveChild(box);
  RemoveChild(box);
  box->DidRemove();
  box->Release();

  if (!parent_node())
    return;

  if (!first_child()) {
    parent_node()->RemoveBox(this);
    return;
  }

  if (first_child() != last_child())
    return;

  // This box has only one child box, move it to parent.
  parent_node()->ReplaceChild(first_child(), this);
  DidRemove();
}

void EditPane::Box::ReplaceContent(ContentWindow*) {
  NOTREACHED();
}

void EditPane::Box::Split(Box*, ContentWindow*, float) {
  NOTREACHED();
}

void EditPane::Box::StopSplitter(const gfx::Point&, Box*) {
  NOTREACHED();
}

void EditPane::Box::WillRemoveChild(Box* child) {
  if (auto const previous_sibling = child->previous_sibling()) {
    previous_sibling->SetBounds(gfx::RectF(
        previous_sibling->origin(), child->bottom_right()));
  } else if (auto const next_sibling = child->next_sibling()) {
    next_sibling->SetBounds(gfx::RectF(
        child->origin(), next_sibling->bottom_right()));
  }
}

// Bounds
void EditPane::Box::DidChangeBounds() {
  Bounds::DidChangeBounds();
  is_content_visible_ = false;
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
    m_pBox->parent_node()->StopSplitter(point, m_pBox);
    Stop();
  }
}

void EditPane::SplitterController::Move(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    m_pBox->parent_node()->MoveSplitter(gfx::PointF(point), m_pBox);
  }
}

void EditPane::SplitterController::Start(State eState, Box& box) {
  DCHECK(!!box.parent_node());
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
      root_box_(new VerticalBox(this)),
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
ContentWindow* EditPane::GetActiveWindow() const {
  auto const box = root_box_->GetActiveLeafBox();
  return box ? box->GetContent() : nullptr;
}

void EditPane::ReplaceActiveContent(ContentWindow* content) {
  DCHECK(!content->is_realized());
  root_box_->GetActiveLeafBox()->ReplaceContent(content);
}

void EditPane::SetContent(ContentWindow* content) {
  DCHECK(!root_box_->first_child());
  auto const box = new LeafBox(this, content);
  root_box_->AppendChild(box);
  box->AddRef();
  if (bounds().empty()) {
    DCHECK(!is_realized());
  } else {
    auto const box_bounds = GetContentsBounds();
    box->SetBounds(box_bounds);
    content->SetBounds(gfx::ToEnclosingRect(box_bounds));
  }

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
  left_box->parent_node()->Split(left_box, new_right_window, right_box_width);
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
  above_box->parent_node()->Split(above_box, new_below_window, below_box_height);
  above_window->MakeSelectionVisible();
}

// ui::Widget
void EditPane::DidChangeBounds() {
  TabContent::DidChangeBounds();
  root_box_->SetBounds(GetContentsBounds());
}

void EditPane::DidHide() {
  TabContent::DidHide();
  root_box_->DidHide();
}

void EditPane::DidRealizeChildWidget(const ui::Widget& window) {
  TabContent::DidRealizeChildWidget(window);
  auto const box = root_box_->FindLeafBoxFromWidget(&window);
  if (!box)
    return;

  auto const next_leaf_box = box->next_sibling() ?
      box->next_sibling()->GetFirstLeafBox() : nullptr;
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
  root_box_->DidShow();
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
  box->parent_node()->RemoveBox(box);
}

// views::TabContent
const domapi::TabData* EditPane::GetTabData() const {
  auto const content = GetActiveWindow();
  if (!content)
    return nullptr;
  return views::TabDataSet::instance()->GetTabData(content->window_id());
}
