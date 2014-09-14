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
  private: Layer* const new_layer_;
  private: std::unique_ptr<Layer> old_layer_;

  protected: ReplaceAnimation(Layer* new_layer,
                              std::unique_ptr<Layer> old_layer);
  protected: virtual ~ReplaceAnimation();

  protected: Layer* new_layer() { return new_layer_; }
  protected: Layer* old_layer() { return old_layer_.get(); }

  protected: void SetUpNewLayer();

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

ReplaceAnimation::~ReplaceAnimation() {
}

void ReplaceAnimation::SetUpNewLayer() {
  DCHECK(!new_layer_->parent_layer());
  old_layer_->parent_layer()->AppendLayer(new_layer_);
}

//////////////////////////////////////////////////////////////////////
//
// ExtendAnimation
//
class ExtendAnimation : public ReplaceAnimation {
  private: std::unique_ptr<AnimationSize> animation_size_;

  public: ExtendAnimation(Layer* new_layer,
                          std::unique_ptr<Layer> old_layer);
  public: virtual ~ExtendAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(ExtendAnimation);
};

ExtendAnimation::ExtendAnimation(Layer* new_layer,
                                 std::unique_ptr<Layer> old_layer)
    : ReplaceAnimation(new_layer, std::move(old_layer)) {
}

void ExtendAnimation::Animate(base::Time now) {
  if (!animation_size_) {
    animation_size_.reset(new AnimationSize(
        now, animation_duration(), old_layer()->bounds().size(),
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
class MoveAnimation : public LayerAnimation {
  private: std::unique_ptr<AnimationPoint> animation_origin_;
  private: Layer* const new_layer_;
  private: const gfx::PointF old_origin_;
  private: Layer* const parent_layer_;

  public: MoveAnimation(Layer* parent_layer, Layer* new_layer,
                        const gfx::PointF& old_origin);
  public: virtual ~MoveAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(MoveAnimation);
};

MoveAnimation::MoveAnimation(Layer* parent_layer, Layer* new_layer,
                             const gfx::PointF& old_origin)
    : new_layer_(new_layer), old_origin_(old_origin),
       parent_layer_(parent_layer) {
  DCHECK(!new_layer_->parent_layer());
  new_layer_->DidRegisterAnimation(this);
}

void MoveAnimation::Animate(base::Time now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(), old_origin_,
        new_layer_->bounds().origin()));
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
class ShrinkAnimation : public ReplaceAnimation {
  private: std::unique_ptr<AnimationSize> animation_size_;

  public: ShrinkAnimation(Layer* new_layer, std::unique_ptr<Layer> old_layer);
  public: virtual ~ShrinkAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(ShrinkAnimation);
};

ShrinkAnimation::ShrinkAnimation(Layer* new_layer,
                                 std::unique_ptr<Layer> old_layer)
    : ReplaceAnimation(new_layer, std::move(old_layer)) {
}

void ShrinkAnimation::Animate(base::Time now) {
  if (!animation_size_) {
    animation_size_.reset(new AnimationSize(
        now, animation_duration(), old_layer()->bounds().size(),
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
// SlideInAnimation
//
class SlideInAnimation : public LayerAnimation {
  private: std::unique_ptr<AnimationPoint> animation_origin_;
  private: Layer* const new_layer_;
  private: Layer* const old_layer_;
  private: Layer* const parent_layer_;

  public: SlideInAnimation(Layer* parent_layer, Layer* new_layer,
                           Layer* old_layer);
  public: virtual ~SlideInAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(SlideInAnimation);
};

SlideInAnimation::SlideInAnimation(Layer* parent_layer, Layer* new_layer,
                                   Layer* old_layer)
    : new_layer_(new_layer), old_layer_(old_layer),
      parent_layer_(parent_layer)  {
  DCHECK(!new_layer_->parent_layer());
  new_layer_->DidRegisterAnimation(this);
  if (!old_layer_)
    return;
  DCHECK_EQ(old_layer_->parent_layer(), parent_layer);
  old_layer_->DidRegisterAnimation(this);
}

void SlideInAnimation::Animate(base::Time now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(),
        gfx::PointF(new_layer_->bounds().left - new_layer_->bounds().width(),
                    new_layer_->bounds().top),
        new_layer_->bounds().origin()));
    parent_layer_->AppendLayer(new_layer_);
  }
  auto const origin = animation_origin_->Compute(now);
  new_layer_->SetOrigin(origin);
  if (origin != animation_origin_->end_value()) {
    ScheduleAnimation();
    return;
  }
  FinalizeAnimation();
}

void SlideInAnimation::FinalizeAnimation() {
  LayerAnimation::FinalizeAnimation();
  if (animation_origin_) {
    new_layer_->SetOrigin(animation_origin_->end_value());
    return;
  }
  parent_layer_->AppendLayer(new_layer_);
  if (!old_layer_)
    return;
  old_layer_->parent_layer()->RemoveLayer(old_layer_);
}

//////////////////////////////////////////////////////////////////////
//
// SlideOutAnimation
//
class SlideOutAnimation : public LayerAnimation {
  private: std::unique_ptr<AnimationPoint> animation_origin_;
  private: std::unique_ptr<Layer> layer_;
  private: const gfx::PointF new_origin_;

  public: SlideOutAnimation(std::unique_ptr<Layer> layer,
                            const gfx::PointF& new_origin);
  public: virtual ~SlideOutAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(SlideOutAnimation);
};

SlideOutAnimation::SlideOutAnimation(std::unique_ptr<Layer> layer,
                                     const gfx::PointF& new_origin)
    : layer_(std::move(layer)), new_origin_(new_origin) {
  DCHECK(layer_->parent_layer());
  layer_->DidRegisterAnimation(this);
}

void SlideOutAnimation::Animate(base::Time now) {
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
class SlideReplaceAnimation : public ReplaceAnimation {
  private: std::unique_ptr<AnimationPoint> animation_origin_;
  private: const gfx::PointF old_origin_;

  public: SlideReplaceAnimation(Layer* new_layer,
                                std::unique_ptr<Layer> old_layer,
                                const gfx::PointF& old_origin);
  public: virtual ~SlideReplaceAnimation() = default;

  // Animatable
  private: virtual void Animate(base::Time now) override;
  private: virtual void FinalizeAnimation() override;

  DISALLOW_COPY_AND_ASSIGN(SlideReplaceAnimation);
};

SlideReplaceAnimation::SlideReplaceAnimation(
    Layer* new_layer, std::unique_ptr<Layer> old_layer,
    const gfx::PointF& old_origin)
    : ReplaceAnimation(new_layer, std::move(old_layer)),
    old_origin_(old_origin) {
}

void SlideReplaceAnimation::Animate(base::Time now) {
  if (!animation_origin_) {
    animation_origin_.reset(new AnimationPoint(
        now, animation_duration(), old_origin_,
        new_layer()->bounds().origin()));
    SetUpNewLayer();
  }
  auto const origin = animation_origin_->Compute(now);
  new_layer()->SetOrigin(origin);
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
LayerAnimation::LayerAnimation() {
}

LayerAnimation::~LayerAnimation() {
}

base::TimeDelta LayerAnimation::animation_duration() const {
  return base::TimeDelta::FromMilliseconds(16 * 10);
}

LayerAnimation* LayerAnimation::CreateExtend(
    Layer* new_layer, std::unique_ptr<Layer> old_layer) {
  return new ExtendAnimation(new_layer, std::move(old_layer));
}

LayerAnimation* LayerAnimation::CreateMove(
    Layer* parent_layer, Layer* new_layer, const gfx::PointF& old_origin) {
  return new MoveAnimation(parent_layer, new_layer, old_origin);
}

LayerAnimation* LayerAnimation::CreateShrink(
    Layer* new_layer, std::unique_ptr<Layer> old_layer) {
  return new ShrinkAnimation(new_layer, std::move(old_layer));
}

LayerAnimation* LayerAnimation::CreateSlideIn(
    Layer* parent_layer, Layer* new_layer, Layer* old_layer) {
  return new SlideInAnimation(parent_layer, new_layer, old_layer);
}

LayerAnimation* LayerAnimation::CreateSlideOut(
    std::unique_ptr<Layer> layer, const gfx::PointF& new_origin) {
  return new SlideOutAnimation(std::move(layer), new_origin);
}

LayerAnimation* LayerAnimation::CreateSlideReplace(
    Layer* new_layer, std::unique_ptr<Layer> old_layer,
    const gfx::PointF& old_origin) {
  return new SlideReplaceAnimation(new_layer, std::move(old_layer), old_origin);
}

}  // namespace ui
