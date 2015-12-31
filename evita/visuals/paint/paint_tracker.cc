// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#include "evita/visuals/paint/paint_tracker.h"

#include "base/logging.h"
#include "base/macros.h"
#include "evita/visuals/geometry/affine_transformer.h"
#include "evita/visuals/model/box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

// Map |Box| to box bounds in viewport coordinate.
using BoundsMap = std::unordered_map<const Box*, FloatRect>;

//////////////////////////////////////////////////////////////////////
//
// PaintTracker::Impl
//
class PaintTracker::Impl final {
 public:
  explicit Impl(const Box& root_box);
  ~Impl();

  void Clear();
  std::vector<FloatRect> Update();

 private:
  struct Context {
    BoundsMap last_bounds_map;
    std::vector<const Box*> changed_boxes;
  };

  class TransformScope {
   public:
    explicit TransformScope(Impl* impl) : impl_(impl) {
      impl_->PushTransform();
    }
    ~TransformScope() { impl_->PopTransform(); }

   private:
    Impl* const impl_;

    DISALLOW_COPY_AND_ASSIGN(TransformScope);
  };

  void PopTransform();
  void PushTransform();
  void Visit(Context* context, const Box& box);

  BoundsMap bounds_map_;
  const Box& root_box_;
  AffineTransformer transformer_;
  std::stack<FloatMatrix3x2> transforms_;

  DISALLOW_COPY_AND_ASSIGN(PaintTracker::Impl);
};

PaintTracker::Impl::Impl(const Box& root_box) : root_box_(root_box) {
  transforms_.push(transformer_.matrix());
}

PaintTracker::Impl::~Impl() {
  DCHECK_EQ(1, transforms_.size());
  transforms_.pop();
}

void PaintTracker::Impl::Clear() {
  bounds_map_.clear();
}

void PaintTracker::Impl::PopTransform() {
  const auto& last_transform = transforms_.top();
  transforms_.pop();
  if (last_transform == transforms_.top())
    return;
  transformer_.set_matrix(transforms_.top());
}

void PaintTracker::Impl::PushTransform() {
  transforms_.push(transformer_.matrix());
}

// TODO(eval1749): We should use cull rect set to merge adjacent rects.
std::vector<FloatRect> PaintTracker::Impl::Update() {
  Context context;
  context.last_bounds_map.swap(bounds_map_);
  Visit(&context, root_box_);
  if (context.last_bounds_map.empty() || context.changed_boxes.empty())
    return std::vector<FloatRect>();

  BoxEditor editor;
  std::vector<FloatRect> exposed_rect_list;
  for (const auto& box : context.changed_boxes) {
    const auto& old_bounds = context.last_bounds_map.find(box)->second;
    // TODO(eval1749): We should use R-Tree to find revealed boxes.
    for (const auto& pair : bounds_map_) {
      const auto& other = pair.first;
      if (box == other)
        continue;
      const auto& exposed_rect = old_bounds.Intersect(pair.second);
      if (exposed_rect.IsEmpty())
        continue;
      // |other| is revealed by |box|'s bounds change.
      editor.SetShouldPaint(const_cast<Box*>(other));
      if (exposed_rect_list.empty() ||
          !exposed_rect_list.back().Contains(exposed_rect)) {
        exposed_rect_list.push_back(exposed_rect);
      }
    }
  }
  return std::move(exposed_rect_list);
}

void PaintTracker::Impl::Visit(Context* context, const Box& box) {
  transformer_.Translate(box.bounds().origin());
  TransformScope outer_scope(this);

  const auto& new_bounds = transformer_.MapRect(FloatRect(box.bounds().size()));
  bounds_map_.insert(std::make_pair(&box, new_bounds));

  const auto& it = context->last_bounds_map.find(&box);
  if (it != context->last_bounds_map.end() &&
      new_bounds.origin() != it->second.origin()) {
    BoxEditor().DidMove(const_cast<Box*>(&box));
    context->changed_boxes.push_back(&box);
  }

  const auto container = box.as<ContainerBox>();
  if (!container || container->bounds().size().IsEmpty())
    return;

  const auto& content_bounds = box.content_bounds();
  transformer_.Translate(content_bounds.origin());
  TransformScope inner_scope(this);
  for (const auto& child : container->child_boxes())
    Visit(context, *child);
}

//////////////////////////////////////////////////////////////////////
//
// PaintTracker
//
PaintTracker::PaintTracker(const Box& root_box) : impl_(new Impl(root_box)) {}

PaintTracker::~PaintTracker() {}

void PaintTracker::Clear() {
  impl_->Clear();
}

std::vector<FloatRect> PaintTracker::Update() {
  return std::move(impl_->Update());
}

}  // namespace visuals
