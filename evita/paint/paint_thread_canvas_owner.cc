// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/paint_thread_canvas_owner.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx/canvas.h"
#include "evita/paint/paint_thread.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner.h"
#include "evita/ui/compositor/layer_owner_delegate.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// PaintThreadCanvasOwner::Impl
//
class PaintThreadCanvasOwner::Impl final : public ui::LayerOwner,
                                           public ui::LayerOwnerDelegate {
 public:
  Impl(PaintThreadCanvasOwner* owner, ui::AnimatableWindow* widget);
  virtual ~Impl();

  void DidBeginAnimationFrame(base::Time now);
  void DidRealize();
  void DidRecreateParentLayer();
  void RequestAnimationFrame();
  void WillDestroyWidget();

 private:
  bool EnsureCanvas();

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  std::unique_ptr<gfx::Canvas> canvas_;
  PaintThreadCanvasOwner* const owner_;
  ui::AnimatableWindow* const window_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

PaintThreadCanvasOwner::Impl::Impl(PaintThreadCanvasOwner* owner,
                                   ui::AnimatableWindow* widget)
    : owner_(owner), window_(widget) {
  set_layer_owner_delegate(this);
}

PaintThreadCanvasOwner::Impl::~Impl() {}

void PaintThreadCanvasOwner::Impl::DidBeginAnimationFrame(base::Time now) {
  TRACE_EVENT_WITH_FLOW0("paint", "MetricsView::DidBeginAnimationFrame", owner_,
                         TRACE_EVENT_FLAG_FLOW_IN);
  if (!EnsureCanvas())
    return window_->RequestAnimationFrame();
  owner_->Paint(canvas_.get(), now);
}

void PaintThreadCanvasOwner::Impl::DidRealize() {
  auto const compositor = paint::PaintThread::instance()->compositor();
  SetLayer(new ui::Layer(compositor));
  window_->layer()->AppendLayer(layer());
  layer()->SetBounds(window_->GetContentsBounds());
  // Note: It is too early to call Compositor::CommitIfNeeded(), since
  // main visual tree isn't committed yet.
}

void PaintThreadCanvasOwner::Impl::DidRecreateParentLayer() {
  window_->layer()->RemoveLayer(layer());
  RecreateLayer();
  window_->layer()->AppendLayer(layer());
}

bool PaintThreadCanvasOwner::Impl::EnsureCanvas() {
  if (!canvas_)
    canvas_.reset(layer()->CreateCanvas());
  else if (canvas_->GetLocalBounds() != window_->GetContentsBounds())
    canvas_->SetBounds(window_->GetContentsBounds());
  return canvas_->IsReady();
}

void PaintThreadCanvasOwner::Impl::RequestAnimationFrame() {
  window_->RequestAnimationFrame();
}

void PaintThreadCanvasOwner::Impl::WillDestroyWidget() {
  DestroyLayer();
}

// Implementation of ui::LayerOwnerDelegate
void PaintThreadCanvasOwner::Impl::DidRecreateLayer(ui::Layer*) {
  canvas_.reset();
}

//////////////////////////////////////////////////////////////////////
//
// PaintThreadCanvasOwner
//
PaintThreadCanvasOwner::PaintThreadCanvasOwner(ui::AnimatableWindow* widget)
    : impl_(new Impl(this, widget)) {}

PaintThreadCanvasOwner::~PaintThreadCanvasOwner() {}

void PaintThreadCanvasOwner::DidBeginAnimationFrame(base::Time now) {
  TRACE_EVENT_WITH_FLOW0("paint",
                         "PaintThreadCanvasOwner::DidBeginAnimationFrame", this,
                         TRACE_EVENT_FLAG_FLOW_OUT);
  paint::PaintThread::instance()->SchedulePaintTask(
      base::Bind(&PaintThreadCanvasOwner::Impl::DidBeginAnimationFrame,
                 base::Unretained(impl_.get()), now));
}

void PaintThreadCanvasOwner::DidRealize() {
  paint::PaintThread::instance()->PostTask(
      FROM_HERE, base::Bind(&PaintThreadCanvasOwner::Impl::DidRealize,
                            base::Unretained(impl_.get())));
}

void PaintThreadCanvasOwner::DidRecreateParentLayer() {
  paint::PaintThread::instance()->PostTask(
      FROM_HERE,
      base::Bind(&PaintThreadCanvasOwner::Impl::DidRecreateParentLayer,
                 base::Unretained(impl_.get())));
}

void PaintThreadCanvasOwner::RequestAnimationFrame() {
  impl_->RequestAnimationFrame();
}

void PaintThreadCanvasOwner::WillDestroyWidget() {
  // TODO(eval1749): We should wait for |PaintThread| to finish painting.
  impl_->WillDestroyWidget();
}

}  // namespace paint
