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
// Caret::Updater
//
Caret::Updater::Updater(Caret* caret) : caret_(caret) {
  caret_->shown_ = false;
}

Caret::Updater::~Updater() {
}

void Caret::Updater::Clear() {
  caret_->bounds_ = gfx::RectF();
}

void Caret::Updater::Update(const gfx::Canvas* gfx,
                            const gfx::RectF& new_rect) {
  caret_->Update(gfx, new_rect);
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret() : owner_(false), shown_(false) {
}

Caret::~Caret() {
  DCHECK(!owner_);
}

void Caret::Blink(const gfx::Canvas* gfx) {
  if (!owner_ || bounds_.empty())
    return;
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  if (shown_)
    Hide(gfx);
  else
    Show(gfx);
}

void Caret::Give(views::Window* owner, const gfx::Canvas* gfx) {
  DCHECK_EQ(owner_, owner);
  Hide(gfx);
  owner_ = nullptr;
}

void Caret::Hide(const gfx::Canvas* gfx) {
  if (!owner_ || !shown_ || bounds_.empty())
    return;
  shown_ = false;
  if (!gfx->screen_bitmap())
    return;
  gfx::Canvas::DrawingScope drawing_scope(*gfx);
  gfx->set_dirty_rect(bounds_);
  gfx::Canvas::AxisAlignedClipScope clip_scope(*gfx, bounds_);
  gfx->DrawBitmap(*gfx->screen_bitmap(), bounds_, bounds_);
}

void Caret::Show(const gfx::Canvas* gfx) {
  DCHECK(owner_);
  if (shown_ || bounds_.empty())
    return;
  shown_ = true;
  gfx::Canvas::DrawingScope drawing_scope(*gfx);
  gfx->set_dirty_rect(bounds_);
  gfx::Canvas::AxisAlignedClipScope clip_scope(*gfx, bounds_);
  gfx::Brush fill_brush(*gfx, gfx::ColorF::Black);
  gfx->FillRectangle(fill_brush, bounds_);
}

void Caret::Take(views::Window* owner) {
  DCHECK(!owner_);
  owner_ = owner;
}

void Caret::Update(const gfx::Canvas* gfx, const gfx::RectF& new_rect) {
  DCHECK(!shown_);
  if (!owner_) {
    bounds_ = new_rect;
    return;
  }
  if (bounds_ != new_rect) {
    bounds_ = new_rect;
    last_blink_time_ = base::Time::Now();
  }
  Show(gfx);
}
