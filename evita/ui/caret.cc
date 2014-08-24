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

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// Caret::Delegate
//
Caret::Delegate::Delegate() {
}

Caret::Delegate::~Delegate() {
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

void Caret::Blink(Delegate* delegate, gfx::Canvas* canvas) {
  if (owner_ != delegate || !bounds_)
    return;
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  if (shown_) {
    owner_->HideCaret(canvas, *this);
    shown_ = false;
  } else {
    owner_->ShowCaret(canvas, *this);
    shown_ = true;
  }
}

void Caret::DidPaint(Delegate* delegate, const gfx::RectF& paint_bounds) {
  DCHECK(!paint_bounds.empty());
  if (owner_ != delegate || bounds_.empty())
    return;
  if (paint_bounds.Intersect(bounds_).empty())
    return;
  // We'll soon update caret bounds. So, we don't reset caret position for
  // |ui::TextInputClient|.
  bounds_ = gfx::RectF();
  last_blink_time_ = base::Time();
  shown_ = false;
}

void Caret::Give(Delegate* owner) {
  DCHECK_EQ(owner_, owner);
  owner_ = nullptr;
  bounds_ = gfx::RectF();
  last_blink_time_ = base::Time();
  shown_ = false;
  ui::TextInputClient::Get()->set_caret_bounds(bounds_);
}

void Caret::Hide(Delegate* delegate) {
  if (owner_ != delegate)
    return;
  DCHECK(!shown_);
  bounds_ = gfx::RectF();
  ui::TextInputClient::Get()->set_caret_bounds(bounds_);
}

void Caret::Take(Delegate* owner) {
  DCHECK(!owner_);
  bounds_ = gfx::RectF();
  owner_ = owner;
  shown_ = false;
  ui::TextInputClient::Get()->set_caret_bounds(bounds_);
}

void Caret::Update(Delegate* delegate, gfx::Canvas* canvas,
                   const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (owner_ != delegate)
    return;
  DCHECK(!shown_);
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  // Don't blink caret while when caret is moved or resized.
  last_blink_time_ = base::Time::Now();
  owner_->ShowCaret(canvas, *this);
  ui::TextInputClient::Get()->set_caret_bounds(bounds_);
  shown_ = true;
}

}  // namespace ui
