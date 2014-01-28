#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Caret.h"

#include "evita/gfx_base.h"
#include <math.h>
#include <utility>

#define DEBUG_BLINK 0
#define DEBUG_DRAW 0
#define DEBUG_SHOW 0

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// BackingStore
//
class Caret::BackingStore {
  private: common::OwnPtr<gfx::Bitmap> bitmap_;
  private: void* render_target_;
  private: gfx::RectF rect_;

  public: BackingStore();
  public: ~BackingStore();
  public: void Restore(const gfx::Graphics& gfx);
  public: void Save(const gfx::Graphics& gfx, const gfx::RectF& rect);
        
  DISALLOW_COPY_AND_ASSIGN(BackingStore);
};

Caret::BackingStore::BackingStore()
    : render_target_(nullptr) {
}

Caret::BackingStore::~BackingStore() {
}

void Caret::BackingStore::Restore(const gfx::Graphics& gfx) {
  if (!rect_ || render_target_ != &*gfx)
    return;
  ASSERT(!!bitmap_);
  gfx->DrawBitmap(*bitmap_, rect_);
  auto const hr = gfx->Flush();
  if (FAILED(hr)) {
    DEBUG_PRINTF("DrawBitmap failed hr=0x%08X\n", hr);
  }
}

void Caret::BackingStore::Save(const gfx::Graphics& gfx, 
                               const gfx::RectF& rect) {
  ASSERT(!rect.is_empty());
  rect_ = gfx::RectF(::floorf(rect.left), ::floorf(rect.top),
                     ::ceilf(rect.right), ::ceilf(rect.bottom));
  gfx::RectU screen_rect = gfx::RectU(static_cast<uint>(rect_.left),
                                      static_cast<uint>(rect_.top),
                                      static_cast<uint>(rect_.right),
                                      static_cast<uint>(rect_.bottom));
  common::OwnPtr<gfx::Bitmap> bitmap(*new gfx::Bitmap(gfx,
                                                    screen_rect.size()));
  COM_VERIFY((*bitmap)->CopyFromRenderTarget(nullptr, gfx, &screen_rect));
  bitmap_ = std::move(bitmap);
  render_target_ = &*gfx;
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : backing_store_(new BackingStore()),
    blink_timer_(this, &Caret::Blink),
    gfx_(nullptr),
    shown_(false),
    should_blink_(false),
    taken_(false) {
}

Caret::~Caret() {
  ASSERT(!taken_);
}

void Caret::Blink(common::RepeatingTimer<Caret>*) {
  if (!taken_ || !should_blink_ || !rect_)
    return;
  #if DEBUG_BLINK
    auto now = ::GetTickCount();
    static decltype(now) last_at;
    DEBUG_PRINTF("show=%d %dms\n", shown_, now - last_at);
    last_at = now;
  #endif
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  if (shown_)
    Hide();
  else
    Show();
}

std::unique_ptr<Caret> Caret::Create() {
  return std::move(std::unique_ptr<Caret>(new Caret()));
}

void Caret::Give() {
  #if DEBUG_SHOW
    DEBUG_PRINTF("gfx=%p\n", gfx_);
  #endif
  ASSERT(taken_);
  blink_timer_.Stop();
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  Hide();
  taken_ = false;
}

void Caret::Hide() {
  #if DEBUG_DRAW || DEBUG_SHOW
    DEBUG_PRINTF("taken=%d shown=%d\n", taken_, shown_);
  #endif
  if (!taken_)
    return;
  if (!shown_)
    return;
  backing_store_->Restore(*gfx_);
  shown_ = false;
}

void Caret::Reset() {
  DCHECK(!shown_);
  should_blink_ = false;
  rect_ = gfx::RectF();
}

void Caret::Show() {
  #if DEBUG_DRAW
    DEBUG_PRINTF("taken=%d shown=%d\n", taken_, shown_);
  #endif
  ASSERT(!!rect_);
  if (shown_)
    return;
  backing_store_->Save(*gfx_, rect_);
  gfx::Brush fill_brush(*gfx_, gfx::ColorF::Black);
  gfx_->FillRectangle(fill_brush, rect_);
  shown_ = true;
}

void Caret::Take(const gfx::Graphics& gfx) {
  #if DEBUG_SHOW
    DEBUG_PRINTF("gfx=%p taken=%d\n", &gfx, taken_);
  #endif
  taken_ = true;
  gfx_ = &gfx;
  blink_timer_.Start(kBlinkInterval);
  if (!rect_)
    return;
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  Show();
}

void Caret::Update(const gfx::RectF& new_rect) {
  ASSERT(!!new_rect);
  ASSERT(taken_);
  ASSERT(!shown_);
  #if DEBUG_SHOW
    DEBUG_PRINTF("Update caret to (%d,%d) from (%d,%d)\n", 
        static_cast<uint>(new_rect.left), static_cast<uint>(new_rect.top),
        static_cast<uint>(rect_.left), static_cast<uint>(rect_.top));
  #endif
  if (rect_ == new_rect) {
    should_blink_ = true;
  } else {
    rect_ = new_rect;
    should_blink_ = false;
  }
  Show();
}
