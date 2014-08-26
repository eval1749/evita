// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/caret.h"

#include <math.h>
#include <memory>
#include <utility>

#include "base/logging.h"
#include "evita/gfx/bitmap.h"
#include "evita/ui/base/ime/text_input_client.h"

namespace ui {

static const auto kBlinkInterval = 16 * 30; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret() : visible_(false) {
}

Caret::~Caret() {
}

void Caret::Blink(gfx::Canvas* canvas) {
  // TODO(eval1749) We should take |last_blink_time_| from a parameter of
  // |ui::Animatable::Animate()|.
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  if (visible_)
    Hide(canvas);
  else
    Show(canvas);
}

void Caret::DidPaint(const gfx::RectF& paint_bounds) {
  DCHECK(!paint_bounds.empty());
  if (bounds_.empty())
    return;
  if (paint_bounds.Intersect(bounds_).empty())
    return;
  // We'll soon update caret bounds. So, we don't reset caret position for
  // |ui::TextInputClient|.
  bounds_ = gfx::RectF();
  visible_ = false;
}

void Caret::Hide(gfx::Canvas* canvas) {
  if (!visible_)
    return;
  auto const bounds = bounds_.Intersect(canvas->bounds());
  if (bounds.empty())
    return;
  visible_ = false;
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  canvas->AddDirtyRect(bounds);
  Paint(canvas, bounds);
}

void Caret::Show(gfx::Canvas* canvas) {
  if (visible_)
    return;
  auto const bounds = bounds_.Intersect(canvas->bounds());
  if (bounds.empty())
    return;
  visible_ = true;
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  canvas->AddDirtyRect(bounds);
  Paint(canvas, bounds);
}

void Caret::Update(gfx::Canvas* canvas, const gfx::RectF& new_bounds) {
  DCHECK(!visible_);
  if (bounds_ != new_bounds) {
    bounds_ = new_bounds;
    // TODO(eval1749) We should take |last_blink_time_| from a parameter of
    // |ui::Animatable::Animate()|.
    last_blink_time_ = base::Time::Now();
    ui::TextInputClient::Get()->set_caret_bounds(bounds_);
  }
  if (bounds_.empty())
    return;
  Show(canvas);
}

}  // namespace ui
