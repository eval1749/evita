#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Caret.h"

#include <math.h>
#include <memory>
#include <utility>

#include "evita/gfx/graphics.h"
#include "evita/gfx/bitmap.h"

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : gfx_(nullptr),
    shown_(false),
    should_blink_(false),
    taken_(false) {
}

Caret::~Caret() {
  DCHECK(!taken_);
}

void Caret::Blink() {
  if (!taken_ || !should_blink_ || !rect_)
    return;
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  gfx_->set_dirty_rect(rect_);
  if (shown_)
    Hide();
  else
    Show();
}

void Caret::Give() {
  DCHECK(taken_);
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  gfx_->set_dirty_rect(rect_);
  Hide();
  taken_ = false;
}

void Caret::Hide() {
  if (!taken_ || !shown_ || rect_.empty())
    return;
  if (gfx_->screen_bitmap())
    gfx_->DrawBitmap(*gfx_->screen_bitmap(), rect_, rect_);
  shown_ = false;
}

void Caret::Reset() {
  DCHECK(!shown_);
  should_blink_ = false;
  rect_ = gfx::RectF();
}

void Caret::Show() {
  DCHECK(rect_);
  DCHECK(taken_);
  if (shown_)
    return;
  gfx::Brush fill_brush(*gfx_, gfx::ColorF::Black);
  gfx_->FillRectangle(fill_brush, rect_);
  shown_ = true;
}

void Caret::Take(const gfx::Graphics& gfx) {
  taken_ = true;
  gfx_ = &gfx;
}

void Caret::Update(const gfx::RectF& new_rect) {
  DCHECK(new_rect);
  DCHECK(!shown_);
  if (!taken_) {
    rect_ = new_rect;
    return;
  }
  if (rect_ == new_rect) {
    should_blink_ = true;
  } else {
    rect_ = new_rect;
    should_blink_ = false;
  }
  Show();
}
