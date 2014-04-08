// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/caret.h"

#include <math.h>
#include <memory>
#include <utility>

#include "base/logging.h"
#include "evita/gfx/bitmap.h"

namespace ui {

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// Caret::Owner
//
Caret::Owner::Owner() {
}

Caret::Owner::~Owner() {
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : owner_(nullptr), shown_(false) {
}

Caret::~Caret() {
  DCHECK(!owner_);
}

void Caret::Blink(gfx::Graphics* gfx) {
  if (!owner_ || !rect_)
    return;
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  shown_ = !shown_;
  last_blink_time_ = now;
  gfx::Graphics::DrawingScope drawing_scope(*gfx);
  gfx->set_dirty_rect(rect_);
  owner_->UpdateCaret(gfx);
}

void Caret::Give(Owner* owner) {
  DCHECK_EQ(owner_, owner);
  owner_ = nullptr;
  rect_ = gfx::RectF();
}

void Caret::Take(Owner* owner) {
  DCHECK(!owner_);
  owner_ = owner;
  rect_ = gfx::RectF();
}

void Caret::Update(gfx::Graphics* gfx, const gfx::RectF& new_rect) {
  DCHECK(owner_);
  DCHECK(new_rect);
  if (rect_ != new_rect) {
    if (!rect_.empty()) {
      // Don't blink caret while when caret is moved or resized.
      last_blink_time_ = base::Time::Now();
    }
    rect_ = new_rect;
    shown_ = true;
  }
  if (!shown_)
    return;
  gfx::Brush fill_brush(*gfx, gfx::ColorF::Black);
  gfx->FillRectangle(fill_brush, rect_);
}

}  // namespace ui
