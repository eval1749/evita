#include "precomp.h"
// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_EditPane.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "common/castable.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants_or_self.h"
#include "common/tree/node.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/resource.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_animation.h"
#include "evita/ui/compositor/layer_owner.h"
#include "evita/ui/events/event.h"
#include "evita/views/content_observer.h"
#include "evita/views/content_window.h"
#include "evita/vi_Frame.h"

extern HINSTANCE g_hInstance;

using namespace views;

namespace {

auto const kMinBoxHeight = 200.0f;
auto const kMinBoxWidth = 200.0f;
auto const kSplitterWidth = 8.0f;
auto const kSplitterHeight = 8.0f;

bool should_not_use_animation;

//////////////////////////////////////////////////////////////////////
//
// Bounds
//
class Bounds {
  private: gfx::RectF bounds_;

  protected: Bounds(const gfx::RectF& bounds);
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
  protected: virtual void WillChangeBounds(const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Bounds);
};

Bounds::Bounds(const gfx::RectF& bounds) : bounds_(bounds) {
  DCHECK_EQ(bounds_, gfx::RectF(gfx::ToEnclosingRect(bounds_)));
}

void Bounds::DidChangeBounds() {
}

void Bounds::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK_GE(new_bounds.width(), kMinBoxWidth);
  DCHECK_GE(new_bounds.height(), kMinBoxHeight);
  DCHECK_EQ(new_bounds, gfx::RectF(gfx::ToEnclosingRect(new_bounds)));
  if (bounds_ == new_bounds)
    return;
  WillChangeBounds(new_bounds);
  bounds_ = new_bounds;
  DidChangeBounds();
}

void Bounds::WillChangeBounds(const gfx::RectF&) {
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
  private: bool is_content_dirty_;
  private: bool is_removed_;

  protected: Box(EditPane* edit_pane, const gfx::RectF& bounds);
  public: virtual ~Box();

  public: EditPane* edit_pane() { return edit_pane_; }
  public: bool is_removed() const { return is_removed_; }

  public: virtual void AddAnimation(ui::Animatable* animatable);
  public: virtual void Destroy();
  public: void DidActivateContent();
  public: virtual void DidHide();
  public: void DidRemove();
  public: virtual void DidShow();
  public: virtual void DidShowContent();
  public: virtual void DidShowChildContent(Box* child);
  public: void EnsureInHorizontalBox();
  public: void EnsureInVerticalBox();
  public: Box* FindBoxFromContent(const ContentWindow* content) const;
  public: Box* GetActiveLeafBox() const;
  public: virtual ContentWindow* GetContent() const;
  public: Box* GetFirstLeafBox() const;
  public: virtual ui::Layer* GetLayer();
  public: virtual ::HitTestResult HitTest(const gfx::PointF& point) const = 0;
  protected: void MarkContentDirty();
  public: virtual void MoveSplitter(const gfx::PointF&, Box*);
  public: virtual void PrepareAnimation(ui::Animator* animator);
  public: virtual void Realize();
  public: void RemoveBox(Box*);
  public: virtual void SetContent(ContentWindow* new_content);
  public: virtual void Split(Box* ref_box, ContentWindow* new_window,
                             float new_box_size);
  public: virtual void StopSplitter(const gfx::Point&, Box*);
  public: virtual void WillDestroyContent();
  public: virtual void WillRemove();
  public: void WillRemoveChild(Box* child);

  // Bounds
  protected: void DidChangeBounds() override;

  DISALLOW_COPY_AND_ASSIGN(Box);
};

namespace {

typedef EditPane::Box Box;

//////////////////////////////////////////////////////////////////////
//
// ContentWatcher
//
class ContentWatcher : public ContentObserver {
  private: Box* box_;
  private: ContentWindow* content_;
  private: bool is_observing_content_;

  public: ContentWatcher(Box* box, ContentWindow* content);
  public: virtual ~ContentWatcher();

  public: void SetContent(ContentWindow* content);
  public: void WillChangeContent();

  // views::ContentObserver
  private: virtual void DidActivateContent(
      ContentWindow* content_window) override;
  private: virtual void DidUpdateContent(
      ContentWindow* content_window) override;
};

ContentWatcher::ContentWatcher(Box* box, ContentWindow* content)
    : box_(box), content_(content), is_observing_content_(false) {
  content_->AddObserver(this);
  WillChangeContent();
}

ContentWatcher::~ContentWatcher() {
  content_->RemoveObserver(this);
}

void ContentWatcher::SetContent(ContentWindow* new_content) {
  DCHECK_NE(content_, new_content);
  content_->RemoveObserver(this);
  content_ = new_content;
  content_->AddObserver(this);
  WillChangeContent();
}

void ContentWatcher::WillChangeContent() {
  if (is_observing_content_)
    return;
  is_observing_content_ = true;
}

// views::ContentObserver
void ContentWatcher::DidActivateContent(views::ContentWindow*) {
  box_->DidActivateContent();
}

void ContentWatcher::DidUpdateContent(views::ContentWindow*) {
  if (!is_observing_content_)
    return;
  is_observing_content_ = false;
  box_->DidShowContent();
}

//////////////////////////////////////////////////////////////////////
//
// HorizontalBox
//
class HorizontalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(HorizontalBox, Box);

  public: HorizontalBox(EditPane* edit_pane, const gfx::RectF& bounds);
  public: virtual ~HorizontalBox() = default;

  // Bounds
  private: virtual void DidChangeBounds() override;

  // EditPane::Box
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

  private: ContentWindow* content_;
  private: ContentWatcher content_watcher_;
  private: std::unique_ptr<ui::Layer> old_layer_;
  private: ui::LayerOwner* old_layer_owner_;
  private: gfx::RectF start_bounds_;
  private: ui::Layer* visible_layer_;

  public: LeafBox(EditPane* edit_pane, const gfx::RectF& bounds,
                  ContentWindow* content);
  public: virtual ~LeafBox();

  // Bounds
  private: void DidChangeBounds() override;
  private: void WillChangeBounds(const gfx::RectF& new_bounds) override;

  // EditPane::Box
  private: virtual void Destroy() override;
  private: virtual void DidHide() override;
  private: virtual void DidShow() override;
  private: virtual void DidShowContent() override;
  private: virtual ContentWindow* GetContent() const override;
  private: virtual ui::Layer* GetLayer() override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Realize() override;
  private: virtual void SetContent(ContentWindow* new_window) override;
  private: virtual void PrepareAnimation(ui::Animator* animator) override;
  private: virtual void WillDestroyContent() override;
  private: virtual void WillRemove() override;

  DISALLOW_COPY_AND_ASSIGN(LeafBox);
};

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
class RootBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(RootBox, Box);

  private: std::vector<ui::Animatable*> animations_;

  public: RootBox(EditPane* edit_pane);
  public: virtual ~RootBox();

  private: const Box* container() const { return first_child(); }
  private: Box* container() { return first_child(); }

  // Bounds
  private: virtual void DidChangeBounds() override;

  // EditPane::Box
  private: virtual void AddAnimation(ui::Animatable* animatable) override;
  private: virtual void Destroy() override;
  private: virtual void DidShowContent() override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void SetContent(ContentWindow* new_content) override;

  DISALLOW_COPY_AND_ASSIGN(RootBox);
};

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
class VerticalBox final : public EditPane::Box {
  DECLARE_CASTABLE_CLASS(VerticalBox, Box);

  public: VerticalBox(EditPane* edit_pane, const gfx::RectF& bounds);
  public: virtual ~VerticalBox() = default;

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
HorizontalBox::HorizontalBox(EditPane* edit_pane, const gfx::RectF& bounds)
    : Box(edit_pane, bounds) {
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
    for (const auto child : child_nodes())
      old_width += child->width() + kSplitterWidth;
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
                          ContentWindow* right_window,
                          float right_box_width) {
  DCHECK(!right_window->is_realized());
  DCHECK_GE(right_box_width, kMinBoxWidth);

  auto const left_box_width = left_box->width() - right_box_width -
                              kSplitterWidth;
  DCHECK_GE(left_box_width, kMinBoxWidth);

  auto right_box = new LeafBox(edit_pane_,
      gfx::RectF(gfx::PointF(right(), top()),
                 gfx::SizeF(right_box_width, height())),
      right_window);
  InsertAfter(right_box, left_box);
  right_box->AddRef();

  left_box->SetBounds(gfx::RectF(
      left_box->origin(), gfx::SizeF(left_box_width, height())));
  right_box->SetBounds(gfx::RectF(
      gfx::PointF(left_box->right() + kSplitterWidth, top()),
      gfx::SizeF(right_box_width, height())));
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
LeafBox::LeafBox(EditPane* edit_pane, const gfx::RectF& bounds,
                 ContentWindow* content)
    : Box(edit_pane, bounds), content_(content),
      content_watcher_(this, content), old_layer_owner_(nullptr),
      start_bounds_(bounds), visible_layer_(nullptr) {
  if (bounds.empty()) {
    // When |EditPane| isn't realized, |bounds| is empty.
    return;
  }
  if (content_->is_realized())
    visible_layer_ = content_->layer();
  content->SetBounds(gfx::ToEnclosingRect(bounds));
}

LeafBox::~LeafBox() {
  DCHECK(!content_);
}

// Bounds
void LeafBox::DidChangeBounds() {
  Box::DidChangeBounds();
  if (!visible_layer_) {
    DCHECK(!old_layer_);
  } else if (old_layer_.get() != visible_layer_) {
    old_layer_owner_ = visible_layer_->owner();
    old_layer_ = old_layer_owner_->RecreateLayer();
    visible_layer_ = old_layer_.get();
    DCHECK(!GetLayer()->parent_layer());
  }
  content_watcher_.WillChangeContent();
  content_->SetBounds(gfx::ToEnclosingRect(bounds()));
}

void LeafBox::WillChangeBounds(const gfx::RectF& new_bounds) {
  Box::WillChangeBounds(new_bounds);
}

// EditPane::Box
void LeafBox::Destroy() {
  visible_layer_ = nullptr;
  old_layer_owner_ = nullptr;
  old_layer_.reset();
  content_->DestroyWidget();
}

void LeafBox::DidHide() {
  Box::DidHide();
  if (visible_layer_)
    visible_layer_->parent_layer()->RemoveLayer(visible_layer_);
  visible_layer_ = nullptr;
  old_layer_owner_ = nullptr;
  old_layer_.reset();
}

void LeafBox::DidShow() {
  DCHECK(!visible_layer_);
  DCHECK(!old_layer_);
  Box::DidShow();
  content_watcher_.WillChangeContent();
}

void LeafBox::DidShowContent() {
  Box::DidShowContent();
}

ContentWindow* LeafBox::GetContent() const {
  return content_;
}

ui::Layer* LeafBox::GetLayer() {
  return content_->layer();
}

HitTestResult LeafBox::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(HitTestResult::Content, this);
}

void LeafBox::PrepareAnimation(ui::Animator* animator) {
  auto const new_layer = GetLayer();
  DCHECK_EQ(new_layer->bounds(), bounds());
  if (visible_layer_ == new_layer)
    return;
  DCHECK(!new_layer->parent_layer());
  auto const parent_layer = parent_node()->GetLayer();
  if (should_not_use_animation) {
    visible_layer_ = new_layer;
    parent_layer->AppendLayer(new_layer);
    old_layer_owner_ = nullptr;
    old_layer_.reset();
    return;
  }
  if (old_layer_) {
    DCHECK(!old_layer_->owner());
    DCHECK_EQ(old_layer_.get(), visible_layer_);
    visible_layer_ = new_layer;

    // |old_layer_| may not have parent layer if it is in animation.
    old_layer_->FinishAnimation();
    DCHECK(old_layer_->parent_layer());

    auto const size_diff = bounds().size() - old_layer_->bounds().size();
    if (!size_diff.height && !size_diff.width) {
      if (old_layer_owner_ != content_) {
        animator->ScheduleAnimation(
            ui::LayerAnimation::CreateSlideReplace(
                new_layer, std::move(old_layer_),
                gfx::PointF(right(), top())));
        return;
      }
      parent_layer->AppendLayer(new_layer);
      old_layer_owner_ = nullptr;
      old_layer_.reset();
      return;
    }

    if (size_diff.height && size_diff.width ||
        origin() != old_layer_->bounds().origin()) {
      // We don't animate for free resizing.
      parent_layer->AppendLayer(new_layer);
      old_layer_owner_ = nullptr;
      old_layer_.reset();
      return;
    }

    if (size_diff.height > 0 || size_diff.width > 0) {
      animator->ScheduleAnimation(ui::LayerAnimation::CreateExtend(
          new_layer, std::move(old_layer_)));
      return;
    }

    animator->ScheduleAnimation(ui::LayerAnimation::CreateShrink(
        new_layer, std::move(old_layer_)));
    return;
  }

  visible_layer_ = new_layer;
  if (origin() == start_bounds_.origin()) {
    parent_layer->AppendLayer(new_layer);
    return;
  }
  animator->ScheduleAnimation(ui::LayerAnimation::CreateMove(
      parent_layer, new_layer, start_bounds_.origin()));
}

void LeafBox::Realize() {
  Box::Realize();
  content_->SetBounds(gfx::ToEnclosingRect(bounds()));
  content_->RealizeWidget();
}

void LeafBox::SetContent(ContentWindow* content) {
  DCHECK(!content->parent_node());
  DCHECK(!content->is_realized());
  if (visible_layer_) {
    // Save current content for replacement animation.
    old_layer_owner_ = visible_layer_->owner();
    old_layer_ = old_layer_owner_->AcquireLayerTree();
  }
  auto const old_content = content_;
  content_ = content;
  content_watcher_.SetContent(content);
  old_content->DestroyWidget();
  MarkContentDirty();
  content_watcher_.WillChangeContent();
  content_->SetBounds(gfx::ToEnclosingRect(bounds()));
}

void LeafBox::WillDestroyContent() {
  scoped_refptr<Box> protect(this);
  parent_node()->RemoveBox(this);
  content_ = nullptr;
}

void LeafBox::WillRemove() {
  Box::WillRemove();
  if (!visible_layer_ || !visible_layer_->owner())
    return;
  auto const sibling = previous_sibling() ? previous_sibling() : next_sibling();
  if (!sibling)
    return;
  old_layer_owner_ = visible_layer_->owner();
  old_layer_ = old_layer_owner_->AcquireLayerTree();
  auto const new_origin = sibling->left() == left() ?
      sibling->top() < top() ? gfx::PointF(left(), bottom())
                             : gfx::PointF(left(), -bottom()) :
      sibling->left() < left() ? gfx::PointF(right(), top()) :
                                 gfx::PointF(-right(), top());
  AddAnimation(ui::LayerAnimation::CreateSlideOut(
      std::move(old_layer_), new_origin));
}

//////////////////////////////////////////////////////////////////////
//
// RootBox
//
RootBox::RootBox(EditPane* edit_pane)
    : Box(edit_pane, gfx::RectF()) {
  auto const container = new VerticalBox(edit_pane, bounds());
  AppendChild(container);
  container->AddRef();
}

RootBox::~RootBox() {
  auto const container = this->container();
  RemoveChild(container);
  container->DidRemove();
  container->Release();
}

// Bounds
void RootBox::DidChangeBounds() {
  container()->SetBounds(bounds());
}

// EditPane::Box
void RootBox::AddAnimation(ui::Animatable* animation) {
  animations_.push_back(animation);
}

void RootBox::Destroy() {
  container()->Destroy();
}

void RootBox::DidShowContent() {
  Box::DidShowContent();
  auto const animator = ui::Animator::instance();
  container()->PrepareAnimation(animator);
  for (auto animation : animations_)
    animator->ScheduleAnimation(animation);
  animations_.clear();
}

HitTestResult RootBox::HitTest(const gfx::PointF& point) const {
  return container()->HitTest(point);
}

void RootBox::SetContent(ContentWindow* content) {
  DCHECK(!container()->first_child());
  auto const box = new LeafBox(edit_pane_, bounds(), content);
  container()->AppendChild(box);
  box->AddRef();
}

//////////////////////////////////////////////////////////////////////
//
// VerticalBox
//
VerticalBox::VerticalBox(EditPane* edit_pane, const gfx::RectF& bounds)
    : Box(edit_pane, bounds) {
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
    for (const auto child : child_nodes())
      old_height += child->height() + kSplitterHeight;
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

  auto const below_box = new LeafBox(edit_pane_,
      gfx::RectF(gfx::PointF(left(), bottom()),
                 gfx::SizeF(width(), below_box_height)),
      below_window);
  InsertAfter(below_box, above_box);
  below_box->AddRef();

  above_box->SetBounds(gfx::RectF(
      above_box->origin(), gfx::SizeF(width(), above_box_height)));
  below_box->SetBounds(gfx::RectF(
      gfx::PointF(left(), above_box->bottom() + kSplitterHeight),
      gfx::SizeF(width(), below_box_height)));
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
EditPane::Box::Box(EditPane* edit_pane, const gfx::RectF& bounds)
    : Bounds(bounds), edit_pane_(edit_pane), is_content_dirty_(true),
      is_removed_(false) {
}

EditPane::Box::~Box() {
  DCHECK(!first_child());
  DCHECK(is_removed_);
  DCHECK(!parent_node());
  DCHECK(!next_sibling());
  DCHECK(!previous_sibling());
}

void EditPane::Box::AddAnimation(ui::Animatable* animation) {
  parent_node()->AddAnimation(animation);
}

void EditPane::Box::Destroy() {
  DCHECK(!is_removed());
  scoped_refptr<Box> protect(this);
  while (auto const box = first_child())
    box->Destroy();
}

void EditPane::Box::DidActivateContent() {
  edit_pane_->NotifyActivateTabContent();
}

void EditPane::Box::DidHide() {
  for (auto const child : child_nodes())
    child->DidHide();
}

void EditPane::Box::DidRemove() {
  DCHECK(!is_removed());
  is_removed_ = true;
}

void EditPane::Box::DidShow() {
  is_content_dirty_ = true;
  for (auto const child : child_nodes())
    child->DidShow();
}

void EditPane::Box::DidShowContent() {
  is_content_dirty_ = false;
  if (parent_node()) {
    parent_node()->DidShowChildContent(this);
    return;
  }
  edit_pane_->NotifyUpdateTabContent();
}

void EditPane::Box::DidShowChildContent(Box*) {
  for (auto const child : child_nodes()) {
    if (child->is_content_dirty_)
      return;
  }
  // All contents in this box are ready.
  is_content_dirty_ = false;
  DidShowContent();
}

void EditPane::Box::EnsureInHorizontalBox() {
  if (parent_node()->is<HorizontalBox>())
    return;

  auto const layout_box = new HorizontalBox(edit_pane_, bounds());
  parent_node()->ReplaceChild(layout_box, this);
  layout_box->Realize();
  layout_box->AppendChild(this);
}

void EditPane::Box::EnsureInVerticalBox() {
  if (parent_node()->is<VerticalBox>())
    return;

  auto const layout_box = new VerticalBox(edit_pane_, bounds());
  parent_node()->ReplaceChild(layout_box, this);
  layout_box->Realize();
  layout_box->AppendChild(this);
}

EditPane::Box* EditPane::Box::FindBoxFromContent(
    const ContentWindow* content) const {
  DCHECK(!is_removed());
  for (const auto child : common::tree::descendants_or_self(this)) {
    if (child->GetContent() == content)
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

ui::Layer* EditPane::Box::GetLayer() {
  return edit_pane_->layer();
}

void EditPane::Box::MarkContentDirty() {
  for (auto runner = this; runner; runner = runner->parent_node()) {
    if (runner->is_content_dirty_)
      break;
    is_content_dirty_ = true;
  }
}

void EditPane::Box::MoveSplitter(const gfx::PointF&, Box*) {
  NOTREACHED();
}

void EditPane::Box::PrepareAnimation(ui::Animator* animator) {
  DCHECK(!GetContent());
  for (auto const child : child_nodes())
    child->PrepareAnimation(animator);
}

void EditPane::Box::Realize() {
  DCHECK(!is_removed());
  DCHECK(!bounds().empty());
  for (auto child : child_nodes())
    child->Realize();
}

void EditPane::Box::RemoveBox(Box* box) {
  DCHECK(!is_removed());

  scoped_refptr<Box> protect(this);

  box->WillRemove();
  RemoveChild(box);
  box->DidRemove();
  box->Release();

  if (!parent_node())
    return;

  if (!parent_node()->parent_node())
    return;

  if (!first_child()) {
    parent_node()->RemoveBox(this);
    return;
  }

  if (first_child() != last_child())
    return;

  // This box has only one child box, move it to parent.
  WillRemove();
  parent_node()->ReplaceChild(first_child(), this);
  DidRemove();
}

void EditPane::Box::SetContent(ContentWindow*) {
  NOTREACHED();
}

void EditPane::Box::Split(Box*, ContentWindow*, float) {
  NOTREACHED();
}

void EditPane::Box::StopSplitter(const gfx::Point&, Box*) {
  NOTREACHED();
}

void EditPane::Box::WillDestroyContent() {
}

void EditPane::Box::WillRemove() {
  auto const parent = parent_node();
  if (!parent)
    return;
  parent->WillRemoveChild(this);
}

void EditPane::Box::WillRemoveChild(Box* child) {
  if (auto const previous_sibling = child->previous_sibling()) {
    previous_sibling->SetBounds(gfx::RectF(
        previous_sibling->origin(), child->bottom_right()));
    return;
  }

  if (auto const next_sibling = child->next_sibling()) {
    next_sibling->SetBounds(gfx::RectF(
        child->origin(), next_sibling->bottom_right()));
    return;
  }
  // |child| is the last child in |this| box.
}

// Bounds
void EditPane::Box::DidChangeBounds() {
  Bounds::DidChangeBounds();
  MarkContentDirty();
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
    : root_box_(new RootBox(this)),
      splitter_controller_(new SplitterController(*this)) {
}

EditPane::~EditPane() {
}

bool EditPane::has_more_than_one_child() const {
  return first_child() != last_child();
}

// Returns the last active Box.
ContentWindow* EditPane::GetActiveContent() const {
  auto const box = root_box_->GetActiveLeafBox();
  return box ? box->GetContent() : nullptr;
}

Frame* EditPane::GetFrame() const {
  return container_widget().as<Frame>();
}

void EditPane::ReplaceActiveContent(ContentWindow* content) {
  DCHECK(!content->is_realized());
  root_box_->GetActiveLeafBox()->SetContent(content);
  content->SetParentWidget(this);
}

void EditPane::SetContent(ContentWindow* content) {
  root_box_->SetContent(content);
  content->SetParentWidget(this);
}

void EditPane::SplitHorizontally(ContentWindow* left_window,
                                 ContentWindow* new_right_window) {
  DCHECK(left_window->is_realized());
  DCHECK_NE(left_window, new_right_window);
  DCHECK(!new_right_window->is_realized());
  auto const left_box = root_box_->FindBoxFromContent(left_window);
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
  AppendChild(new_right_window);
  new_right_window->RealizeWidget();
  left_window->MakeSelectionVisible();
}

void EditPane::SplitVertically(ContentWindow* above_window,
                               ContentWindow* new_below_window) {
  DCHECK(above_window->is_realized());
  DCHECK_NE(above_window, new_below_window);
  DCHECK(!new_below_window->is_realized());
  auto const above_box = root_box_->FindBoxFromContent(above_window);
  DCHECK(above_box);

  auto const above_box_height = ::floor(
    (above_box->bounds().height() - kSplitterHeight) / 2);
  auto const below_box_height = above_box->height() - above_box_height;
  if (above_box_height < kMinBoxHeight || below_box_height < kMinBoxHeight) {
    GetFrame()->AddOrActivateTabContent(new_below_window);
    return;
  }

  above_box->EnsureInVerticalBox();
  above_box->parent_node()->Split(above_box, new_below_window,
                                  below_box_height);
  AppendChild(new_below_window);
  new_below_window->RealizeWidget();
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

void EditPane::DidRealize() {
  root_box_->SetBounds(GetContentsBounds());
  TabContent::DidRealize();
}

void EditPane::DidRealizeChildWidget(ui::Widget* window) {
  TabContent::DidRealizeChildWidget(window);
  auto const content = window->as<ContentWindow>();
  if (!content)
    return;
  auto const box = root_box_->FindBoxFromContent(content);
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

void EditPane::DidRemoveChildWidget(Widget*) {
  for (auto const child : child_nodes()) {
    if (child->is<ContentWindow>())
      return;
  }
  // There is no window in this pane. So, we delete this pane.
  DestroyWidget();
}

void EditPane::DidSetFocus(ui::Widget*) {
  auto const widget = GetActiveContent();
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
  root_box_->DidRemove();
  root_box_ = nullptr;
}

void EditPane::WillRemoveChildWidget(Widget* old_child) {
  TabContent::WillRemoveChildWidget(old_child);
  auto const content = old_child->as<ContentWindow>();
  if (!content)
    return;
  auto const box = root_box_->FindBoxFromContent(content);
  if (!box)
    return;
  box->WillDestroyContent();
}

// views::TabContent
void EditPane::DidEnterSizeMove() {
  should_not_use_animation = true;
}

void EditPane::DidExitSizeMove() {
  should_not_use_animation = false;
}

const domapi::TabData* EditPane::GetTabData() const {
  auto const content = GetActiveContent();
  if (!content)
    return nullptr;
  return views::TabDataSet::instance()->GetTabData(content->window_id());
}
