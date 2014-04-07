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
  DCHECK(gfx);
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
#if 1
  COM_VERIFY((*bitmap)->CopyFromRenderTarget(nullptr, *gfx, &screen_rect));
#else
  common::ComPtr<ID2D1DeviceContext> context;
  COM_VERIFY(context.QueryFrom(*gfx));
  common::ComPtr<ID2D1Image> image;
  context->GetTarget(&image);
  common::ComPtr<ID2D1Bitmap> src_bitmap;
  COM_VERIFY(src_bitmap.QueryFrom(image));
  COM_VERIFY((*bitmap)->CopyFromBitmap(nullptr, src_bitmap, &screen_rect));
#endif
  bitmap_ = std::move(bitmap);
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : gfx_(nullptr), shown_(false), stop_blinking_(false) {
}

Caret::~Caret() {
  DCHECK(!gfx_);
}

void Caret::Blink(const gfx::Graphics* gfx) {
  if (gfx_ != gfx || !rect_)
    return;
  if (stop_blinking_) {
    if (!shown_) {
      gfx::Graphics::DrawingScope drawing_scope(*gfx_);
      gfx_->set_dirty_rect(rect_);
      Hide(gfx_);
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
  gfx_->set_dirty_rect(rect_);
  if (shown_)
    Hide(gfx_);
  else
    Show(gfx_);
}

void Caret::Give(const gfx::Graphics* gfx) {
  DCHECK_EQ(gfx_, gfx);
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  gfx_->set_dirty_rect(rect_);
  Hide(gfx);
  const_cast<gfx::Graphics*>(gfx_)->RemoveObserver(this);
  gfx_ = nullptr;
}

void Caret::Hide(const gfx::Graphics* gfx) {
  if (gfx_ != gfx || !shown_)
    return;
  if (backing_store_)
    backing_store_->Restore(gfx_);
  shown_ = false;
}

void Caret::Hide() {
  DCHECK(gfx_);
  if (!shown_)
    return;
  shown_ = false;
}

void Caret::Show(const gfx::Graphics* gfx) {
  DCHECK_EQ(gfx_, gfx);
  DCHECK(rect_);
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
  gfx_ = gfx;
  const_cast<gfx::Graphics*>(gfx_)->AddObserver(this);
}

void Caret::Update(const gfx::Graphics* gfx, const gfx::RectF& new_rect) {
  if (gfx_ != gfx)
    return;
  DCHECK(new_rect);
  DCHECK(!shown_);
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
