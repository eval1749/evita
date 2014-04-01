// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/caret.h"

#include <math.h>
#include <memory>
#include <utility>

#include "evita/gfx/bitmap.h"

namespace ui {

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// BackingStore
//
class Caret::BackingStore {
  private: std::unique_ptr<gfx::Bitmap> bitmap_;
  private: gfx::RectF rect_;

  public: BackingStore();
  public: ~BackingStore();
  public: void Restore(const gfx::Graphics* gfx);
  public: void Save(const gfx::Graphics* gfx, const gfx::RectF& rect);

  DISALLOW_COPY_AND_ASSIGN(BackingStore);
};

Caret::BackingStore::BackingStore() {
}

Caret::BackingStore::~BackingStore() {
}

void Caret::BackingStore::Restore(const gfx::Graphics* gfx) {
  if (!rect_ || !bitmap_)
    return;
  (*gfx)->DrawBitmap(*bitmap_, rect_);
  gfx->Flush();
}

void Caret::BackingStore::Save(const gfx::Graphics* gfx,
                               const gfx::RectF& rect) {
  DCHECK(!rect.empty());
  rect_ = gfx::RectF(::floorf(rect.left), ::floorf(rect.top),
                     ::ceilf(rect.right), ::ceilf(rect.bottom));
  gfx::RectU screen_rect = gfx::RectU(static_cast<uint32_t>(rect_.left),
                                      static_cast<uint32_t>(rect_.top),
                                      static_cast<uint32_t>(rect_.right),
                                      static_cast<uint32_t>(rect_.bottom));
  auto bitmap = std::make_unique<gfx::Bitmap>(*gfx, screen_rect.size());
  if (!bitmap || !*bitmap)
    return;
  COM_VERIFY((*bitmap)->CopyFromRenderTarget(nullptr, *gfx, &screen_rect));
  bitmap_ = std::move(bitmap);
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : gfx_(nullptr),
    shown_(false),
    stop_blinking_(false),
    taken_(false) {
}

Caret::~Caret() {
  DCHECK(!taken_);
}

void Caret::Blink(const gfx::Graphics* gfx) {
  if (!taken_ || !rect_)
    return;
  if (stop_blinking_) {
    if (!shown_) {
      gfx::Graphics::DrawingScope drawing_scope(*gfx_);
      Hide(gfx);
    }
    backing_store_.reset();
    return;
  }
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  if (shown_)
    Hide(gfx);
  else
    Show(gfx);
}

void Caret::Give(const gfx::Graphics* gfx) {
  DCHECK_EQ(gfx_, gfx);
  DCHECK(taken_);
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  Hide(gfx);
  taken_ = false;
  const_cast<gfx::Graphics*>(gfx_)->RemoveObserver(this);
  gfx_ = nullptr;
}

void Caret::Hide(const gfx::Graphics* gfx) {
  DCHECK_EQ(gfx_, gfx);
  if (!taken_ || !shown_)
    return;
  if (backing_store_)
    backing_store_->Restore(gfx_);
  shown_ = false;
}

void Caret::Hide() {
  if (!taken_ || !shown_)
    return;
  shown_ = false;
}

void Caret::Show(const gfx::Graphics* gfx) {
  DCHECK_EQ(gfx_, gfx);
  DCHECK(rect_);
  DCHECK(taken_);
  if (shown_)
    return;
  if (!backing_store_)
    backing_store_.reset(new BackingStore());
  backing_store_->Save(gfx_, rect_);
  gfx::Brush fill_brush(*gfx_, gfx::ColorF::Black);
  gfx_->FillRectangle(fill_brush, rect_);
  shown_ = true;
}

void Caret::StopBlinking() {
  stop_blinking_ = true;
  rect_ = gfx::RectF();
}

void Caret::StartBlinking() {
  stop_blinking_ = false;
  last_blink_time_ = base::Time();
}

void Caret::Take(const gfx::Graphics* gfx) {
  DCHECK(!gfx_);
  taken_ = true;
  gfx_ = gfx;
  const_cast<gfx::Graphics*>(gfx_)->AddObserver(this);
}

void Caret::Update(const gfx::Graphics* gfx, const gfx::RectF& new_rect) {
  DCHECK(new_rect);
  DCHECK(!shown_);
  if (!taken_) {
    rect_ = new_rect;
    return;
  }
  stop_blinking_ = false;
  if (rect_ != new_rect) {
    if (!rect_.empty()) {
      // Don't blink caret while when caret is moved or resized.
      last_blink_time_ = base::Time::Now();
    }
    rect_ = new_rect;
  }
  Show(gfx);
}

// gfx::Graphics::Observer
void Caret::ShouldDiscardResources() {
  backing_store_.reset();
}

}  // namespace ui
