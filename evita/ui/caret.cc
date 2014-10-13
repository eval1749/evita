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
#include "evita/ui/caret_owner.h"

namespace ui {

static const auto kBlinkInterval = 16 * 30; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret(CaretOwner* owner) : owner_(owner), visible_(false) {
}

Caret::~Caret() {
}

void Caret::Blink(gfx::Canvas* canvas, base::Time now) {
  auto const bounds = bounds_.Intersect(canvas->GetLocalBounds());
  if (bounds.empty())
    return;

  auto const delta = now - last_blink_time_;
  auto const shape = delta / base::TimeDelta::FromMilliseconds(kBlinkInterval);
  auto const new_visible = !(shape & 1);

  if (visible_ == new_visible)
    return;

  visible_ = new_visible;
  Paint(canvas, bounds);
}

void Caret::DidChangeCaret() {
}

void Caret::DidFireTimer() {
  owner_->DidFireCaretTimer();
}

void Caret::DidPaint(const gfx::RectF& paint_bounds) {
  if (!visible_)
    return;
  DCHECK(!bounds_.empty());
  if (paint_bounds.Intersect(bounds_).empty())
    return;
  // We'll soon update caret bounds. So, we don't reset caret position for
  // |ui::TextInputClient|.
  visible_ = false;
  MakeEmpty();
}

void Caret::Hide(gfx::Canvas* canvas) {
  if (!visible_)
    return;
  visible_ = false;
  auto const bounds = bounds_.Intersect(canvas->GetLocalBounds());
  if (!bounds.empty())
    Paint(canvas, bounds);
  MakeEmpty();
}

void Caret::MakeEmpty() {
  DCHECK(!visible_);
  bounds_ = gfx::RectF();
  last_blink_time_ = base::Time();
  timer_.Stop();
  DidChangeCaret();
}

void Caret::Update(gfx::Canvas* canvas, base::Time now,
                   const gfx::RectF& new_bounds) {
  DCHECK(!visible_);
  if (bounds_ != new_bounds) {
    bounds_ = new_bounds;
    timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(kBlinkInterval),
                 base::Bind(&Caret::DidFireTimer, base::Unretained(this)));
    ui::TextInputClient::Get()->set_caret_bounds(bounds_);
    DidChangeCaret();
  }
  auto const bounds = bounds_.Intersect(canvas->GetLocalBounds());
  if (bounds.empty())
    return;
  last_blink_time_ = now;
  visible_ = true;
  Paint(canvas, bounds);
}

}  // namespace ui
