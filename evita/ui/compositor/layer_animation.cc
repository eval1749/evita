// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/compositor/layer_animation.h"

#include <memory>

#include "base/logging.h"
#include "evita/ui/animation/animation_value.h"
#include "evita/ui/compositor/layer.h"

namespace ui {

namespace {

class ReplaceAnimation : public LayerAnimation {
 protected:
  ReplaceAnimation(Layer* new_layer, std::unique_ptr<Layer> old_layer);
  ~ReplaceAnimation() override;

  Layer* new_layer() { return new_layer_; }
  Layer* old_layer() { return old_layer_.get(); }

  void SetUpNewLayer();

 private:
  Layer* const new_layer_;
  std::unique_ptr<Layer> old_layer_;

  DISALLOW_COPY_AND_ASSIGN(ReplaceAnimation);
};

ReplaceAnimation::ReplaceAnimation(Layer* new_layer,
                                   std::unique_ptr<Layer> old_layer)
    : new_layer_(new_layer), old_layer_(std::move(old_layer)) {
  new_layer_->DidRegisterAnimation(this);
  old_layer_->DidRegisterAnimation(this);
  DCHECK(!new_layer_->parent_layer());
  DCHECK(old_layer_->parent_layer());
}

ReplaceAnimation::~ReplaceAnimation() {}

void ReplaceAnimation::SetUpNewLayer() {
  DCHECK(!new_layer_->parent_layer());
  old_layer_->parent_layer()->AppendLayer(new_layer_);
}

//////////////////////////////////////////////////////////////////////
//
// ExtendAnimation
//
class ExtendAnimation final : public ReplaceAnimation {
 public:
  ExtendAnimation(Layer* new_layer, std::unique_ptr<Layer> old_layer);
  ~ExtendAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationSize> animation_size_;

  DISALLOW_COPY_AND_ASSIGN(ExtendAnimation);
};

ExtendAnimation::ExtendAnimation(Layer* new_layer,
                                 std::unique_ptr<Layer> old_layer)
    : ReplaceAnimation(new_layer, std::move(old_layer)) {}

void ExtendAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_size_) {
    animation_size_.reset(new AnimationSize(now, animation_duration(),
                                            old_layer()->bounds().size(),
                                            new_layer()->bounds().size()));
    SetUpNewLayer();
  }
  const auto size = animation_size_->Compute(now);
  new_layer()->SetClip(gfx::RectF(new_layer()->bounds().origin(), size));
  if (size != animation_size_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void ExtendAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (animation_size_) {
    new_layer()->RemoveClip();
    return;
  }
  SetUpNewLayer();
}

//////////////////////////////////////////////////////////////////////
//
// MoveAnimation
//
class MoveAnimation final : public LayerAnimation {
 public:
  MoveAnimation(Layer* parent_layer,
                Layer* new_layer,
                const gfx::PointF& old_origin);
  ~MoveAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationPoint> animation_origin_;
  Layer* const new_layer_;
  const gfx::PointF old_origin_;
  Layer* const parent_layer_;

  DISALLOW_COPY_AND_ASSIGN(MoveAnimation);
};

MoveAnimation::MoveAnimation(Layer* parent_layer,
                             Layer* new_layer,
                             const gfx::PointF& old_origin)
    : new_layer_(new_layer),
      old_origin_(old_origin),
      parent_layer_(parent_layer) {
  DCHECK(!new_layer_->parent_layer());
  new_layer_->DidRegisterAnimation(this);
}

void MoveAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(), old_origin_, new_layer_->bounds().origin()));
    parent_layer_->AppendLayer(new_layer_);
  }
  const auto origin = animation_origin_->Compute(now);
  new_layer_->SetOrigin(origin);
  if (origin != animation_origin_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void MoveAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (animation_origin_) {
    DCHECK(new_layer_->parent_layer());
    new_layer_->SetOrigin(animation_origin_->end_value());
    return;
  }
  DCHECK(!new_layer_->parent_layer());
  parent_layer_->AppendLayer(new_layer_);
}

//////////////////////////////////////////////////////////////////////
//
// ShrinkAnimation
//
// Shrinks |old_layer_| until |old_layer_| equals to |new_layer_|.
//
class ShrinkAnimation final : public ReplaceAnimation {
 public:
  ShrinkAnimation(Layer* new_layer, std::unique_ptr<Layer> old_layer);
  ~ShrinkAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationSize> animation_size_;

  DISALLOW_COPY_AND_ASSIGN(ShrinkAnimation);
};

ShrinkAnimation::ShrinkAnimation(Layer* new_layer,
                                 std::unique_ptr<Layer> old_layer)
    : ReplaceAnimation(new_layer, std::move(old_layer)) {}

void ShrinkAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_size_) {
    animation_size_.reset(new AnimationSize(now, animation_duration(),
                                            old_layer()->bounds().size(),
                                            new_layer()->bounds().size()));
  }
  const auto size = animation_size_->Compute(now);
  old_layer()->SetClip(gfx::RectF(new_layer()->origin(), size));
  if (size != animation_size_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void ShrinkAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  SetUpNewLayer();
}

//////////////////////////////////////////////////////////////////////
//
// SimpleAnimation
// Moves and changes size of |new_layer| from |old_layer|.
//
class SimpleAnimation final : public ReplaceAnimation {
 public:
  SimpleAnimation(Layer* new_layer, std::unique_ptr<Layer> old_layer);
  ~SimpleAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationPoint> animation_origin_;
  std::unique_ptr<AnimationSize> animation_size_;

  DISALLOW_COPY_AND_ASSIGN(SimpleAnimation);
};

SimpleAnimation::SimpleAnimation(Layer* new_layer,
                                 std::unique_ptr<Layer> old_layer)
    : ReplaceAnimation(new_layer, std::move(old_layer)) {}

void SimpleAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_size_) {
    animation_origin_.reset(new AnimationPoint(now, animation_duration(),
                                               old_layer()->bounds().origin(),
                                               new_layer()->bounds().origin()));
    animation_size_.reset(new AnimationSize(now, animation_duration(),
                                            old_layer()->bounds().size(),
                                            new_layer()->bounds().size()));
    SetUpNewLayer();
  }
  const auto origin = animation_origin_->Compute(now);
  const auto size = animation_size_->Compute(now);
  new_layer()->SetOrigin(origin);
  new_layer()->SetClip(gfx::RectF(size));
  old_layer()->SetClip(gfx::RectF(size));
  if (size != animation_size_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void SimpleAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (animation_size_) {
    new_layer()->RemoveClip();
    return;
  }
  SetUpNewLayer();
}

//////////////////////////////////////////////////////////////////////
//
// SlideInAnimation
// Push out |old_layer_| from |parent_layer_| by moving |new_layer| from
// right to left.
//
//      +-----+      +-----+
//      |     |      |     |
// <==  | old |  <== | new |
//      |     |      |     |
//      +-----+      +-----+
//
class SlideInAnimation final : public LayerAnimation {
 public:
  SlideInAnimation(Layer* parent_layer, Layer* new_layer, Layer* old_layer);
  ~SlideInAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationPoint> animation_origin_;
  Layer* const new_layer_;
  Layer* const old_layer_;
  Layer* const parent_layer_;

  DISALLOW_COPY_AND_ASSIGN(SlideInAnimation);
};

SlideInAnimation::SlideInAnimation(Layer* parent_layer,
                                   Layer* new_layer,
                                   Layer* old_layer)
    : new_layer_(new_layer),
      old_layer_(old_layer),
      parent_layer_(parent_layer) {
  DCHECK(!new_layer_->parent_layer());
  new_layer_->DidRegisterAnimation(this);
  if (!old_layer_)
    return;
  DCHECK_EQ(old_layer_->parent_layer(), parent_layer);
  DCHECK_EQ(old_layer_->origin(), new_layer_->origin());
  old_layer_->DidRegisterAnimation(this);
}

void SlideInAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(),
        gfx::PointF(new_layer_->bounds().right, new_layer_->bounds().top),
        new_layer_->bounds().origin()));
    parent_layer_->AppendLayer(new_layer_);
    new_layer_->SetOrigin(animation_origin_->Compute(now));
  }
  auto const origin = animation_origin_->Compute(now);
  if (old_layer_) {
    auto const diff = origin - new_layer_->origin();
    old_layer_->SetOrigin(old_layer_->origin() + diff);
  }
  new_layer_->SetOrigin(origin);
  if (origin != animation_origin_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void SlideInAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  new_layer_->SetOrigin(animation_origin_->end_value());
  if (old_layer_) {
    old_layer_->parent_layer()->RemoveLayer(old_layer_);
    // Restore |old_layer_| to position before animation.
    old_layer_->SetOrigin(new_layer_->origin());
  }
  if (animation_origin_)
    return;
  parent_layer_->AppendLayer(new_layer_);
}

//////////////////////////////////////////////////////////////////////
//
// SlideOutAnimation
//
class SlideOutAnimation final : public LayerAnimation {
 public:
  SlideOutAnimation(std::unique_ptr<Layer> layer,
                    const gfx::PointF& new_origin);
  ~SlideOutAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationPoint> animation_origin_;
  std::unique_ptr<Layer> layer_;
  const gfx::PointF new_origin_;

  DISALLOW_COPY_AND_ASSIGN(SlideOutAnimation);
};

SlideOutAnimation::SlideOutAnimation(std::unique_ptr<Layer> layer,
                                     const gfx::PointF& new_origin)
    : layer_(std::move(layer)), new_origin_(new_origin) {
  DCHECK(layer_->parent_layer());
  layer_->DidRegisterAnimation(this);
}

void SlideOutAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(), layer_->bounds().origin(), new_origin_));
  }
  const auto origin = animation_origin_->Compute(now);
  layer_->SetOrigin(origin);
  if (origin != animation_origin_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void SlideOutAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (!animation_origin_)
    return;
  layer_->SetOrigin(animation_origin_->end_value());
}

//////////////////////////////////////////////////////////////////////
//
// SlideReplaceAnimation
//
class SlideReplaceAnimation final : public ReplaceAnimation {
 public:
  SlideReplaceAnimation(Layer* new_layer,
                        std::unique_ptr<Layer> old_layer,
                        const gfx::PointF& old_origin);
  ~SlideReplaceAnimation() final = default;

 private:
  // Animatable
  void Animate(const base::TimeTicks& now) final;
  void FinalizeAnimation() final;

  std::unique_ptr<AnimationPoint> animation_origin_;
  const gfx::PointF old_origin_;

  DISALLOW_COPY_AND_ASSIGN(SlideReplaceAnimation);
};

SlideReplaceAnimation::SlideReplaceAnimation(Layer* new_layer,
                                             std::unique_ptr<Layer> old_layer,
                                             const gfx::PointF& old_origin)
    : ReplaceAnimation(new_layer, std::move(old_layer)),
      old_origin_(old_origin) {}

void SlideReplaceAnimation::Animate(const base::TimeTicks& now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(now, animation_duration(),
                                               old_origin_,
                                               new_layer()->bounds().origin()));
    SetUpNewLayer();
    new_layer()->SetOrigin(animation_origin_->Compute(now));
  }
  auto const origin = animation_origin_->Compute(now);
  auto const diff = origin - new_layer()->origin();
  new_layer()->SetOrigin(origin);
  old_layer()->SetOrigin(old_layer()->origin() + diff);
  if (origin != animation_origin_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void SlideReplaceAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (animation_origin_) {
    new_layer()->SetOrigin(animation_origin_->end_value());
    return;
  }
  SetUpNewLayer();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LayerAnimation
//
LayerAnimation::LayerAnimation() {}

LayerAnimation::~LayerAnimation() {}

base::TimeDelta LayerAnimation::animation_duration() const {
#if _DEBUG
  auto const number_of_frames = 30;
#else
  auto const number_of_frames = 13;
#endif
  return base::TimeDelta::FromMilliseconds(16 * number_of_frames);
}

LayerAnimation* LayerAnimation::CreateExtend(Layer* new_layer,
                                             std::unique_ptr<Layer> old_layer) {
  return new ExtendAnimation(new_layer, std::move(old_layer));
}

LayerAnimation* LayerAnimation::CreateMove(Layer* parent_layer,
                                           Layer* new_layer,
                                           const gfx::PointF& old_origin) {
  return new MoveAnimation(parent_layer, new_layer, old_origin);
}

LayerAnimation* LayerAnimation::CreateShrink(Layer* new_layer,
                                             std::unique_ptr<Layer> old_layer) {
  return new ShrinkAnimation(new_layer, std::move(old_layer));
}

LayerAnimation* LayerAnimation::CreateSimple(Layer* new_layer,
                                             std::unique_ptr<Layer> old_layer) {
  return new SimpleAnimation(new_layer, std::move(old_layer));
}

LayerAnimation* LayerAnimation::CreateSlideIn(Layer* parent_layer,
                                              Layer* new_layer,
                                              Layer* old_layer) {
  return new SlideInAnimation(parent_layer, new_layer, old_layer);
}

LayerAnimation* LayerAnimation::CreateSlideOut(std::unique_ptr<Layer> layer,
                                               const gfx::PointF& new_origin) {
  return new SlideOutAnimation(std::move(layer), new_origin);
}

LayerAnimation* LayerAnimation::CreateSlideReplace(
    Layer* new_layer,
    std::unique_ptr<Layer> old_layer,
    const gfx::PointF& old_origin) {
  return new SlideReplaceAnimation(new_layer, std::move(old_layer), old_origin);
}

}  // namespace ui
