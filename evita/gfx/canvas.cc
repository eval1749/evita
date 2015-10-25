// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <d2d1_2helper.h>

#include <cmath>
#include <utility>

#include "evita/gfx/canvas.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas_observer.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/swap_chain.h"
#include "evita/gfx/text_format.h"

#pragma comment(lib, "d3d11.lib")

std::ostream& operator<<(std::ostream& ostream, D2D1_TEXT_ANTIALIAS_MODE mode) {
  switch (mode) {
    case D2D1_TEXT_ANTIALIAS_MODE_DEFAULT:
      return ostream << "DEFAULT";
    case D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE:
      return ostream << "CLEARTYPE";
    case D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE:
      return ostream << "GRAYSCALE";
    case D2D1_TEXT_ANTIALIAS_MODE_ALIASED:
      return ostream << "ALIASED";
  }
  return ostream << "UNKNOWN_" << static_cast<int>(mode);
}

namespace gfx {

namespace {
int global_bitmap_id;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Canvas::AxisAlignedClipScope
//
Canvas::AxisAlignedClipScope::AxisAlignedClipScope(
    Canvas* canvas,
    const RectF& bounds,
    D2D1_ANTIALIAS_MODE alias_mode)
    : canvas_(canvas) {
  DCHECK(!bounds.empty());
  (*canvas_)->PushAxisAlignedClip(bounds, alias_mode);
}

Canvas::AxisAlignedClipScope::AxisAlignedClipScope(Canvas* canvas,
                                                   const RectF& bounds)
    : AxisAlignedClipScope(canvas, bounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE) {}

Canvas::AxisAlignedClipScope::~AxisAlignedClipScope() {
  (*canvas_)->PopAxisAlignedClip();
}

//////////////////////////////////////////////////////////////////////
//
// Canvas::DrawingScope
//
Canvas::DrawingScope::DrawingScope(Canvas* canvas) : canvas_(canvas) {
  canvas_->BeginDraw();
}

Canvas::DrawingScope::~DrawingScope() {
  // TODO(eval1749): Should DrawingScope take mutable Canvas?
  canvas_->EndDraw();
}

//////////////////////////////////////////////////////////////////////
//
// Canvas::ScopedState
//
Canvas::ScopedState::ScopedState(Canvas* canvas)
    : bounds_(canvas->bounds_), canvas_(canvas), offset_(canvas->offset_) {
  (*canvas_)->GetTransform(&transform_);
}

Canvas::ScopedState::~ScopedState() {
  canvas_->bounds_ = bounds_;
  canvas_->offset_ = offset_;
  (*canvas_)->SetTransform(transform_);
}

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
Canvas::Canvas()
    : batch_nesting_level_(0),
      bitmap_id_(0),
      factory_set_(FactorySet::instance()),
      should_clear_(true) {}

Canvas::~Canvas() {}

void Canvas::AddDirtyRect(const RectF& local_bounds) {
  auto const bounds =
      gfx::RectF(local_bounds.origin() + bounds_.origin(), local_bounds.size())
          .Intersect(bounds_);
  if (bounds.empty())
    return;
  AddDirtyRectImpl(bounds);
}

void Canvas::AddDirtyRectImpl(const RectF&) {}

void Canvas::AddObserver(CanvasObserver* observer) {
  observers_.AddObserver(observer);
}

void Canvas::BeginDraw() {
  DCHECK(!bounds_.empty());
  DCHECK(GetRenderTarget());
  TRACE_EVENT_BEGIN1("gfx", "Canvas/Batch", "level", batch_nesting_level_);
  if (!batch_nesting_level_)
    GetRenderTarget()->BeginDraw();
  ++batch_nesting_level_;
}

void Canvas::Clear(const ColorF& color) {
  GetRenderTarget()->Clear(color);
}

void Canvas::Clear(D2D1::ColorF::Enum name) {
  Clear(ColorF(name));
}

void Canvas::CommitDraw() {
  TRACE_EVENT0("gfx", "Canvas::CommitDraw");
  auto const hr = GetRenderTarget()->EndDraw();
  if (SUCCEEDED(hr)) {
    DidCallEndDraw();
    should_clear_ = false;
    return;
  }

  if (hr == D2DERR_RECREATE_TARGET) {
    DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
    DidLostRenderTarget();
    return;
  }

  DVLOG(0) << "ID2D1RenderTarget::Flush: hr=" << std::hex << hr;
}

void Canvas::DidCallEndDraw() {}

void Canvas::DidCreateRenderTarget() {
  bitmap_id_ = ++global_bitmap_id;
  should_clear_ = true;
  SizeF dpi;
  GetRenderTarget()->GetDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
  FOR_EACH_OBSERVER(CanvasObserver, observers_, DidRecreateCanvas());
}

void Canvas::DrawBitmap(const Bitmap& bitmap,
                        const RectF& dst_rect,
                        const RectF& src_rect,
                        float opacity,
                        D2D1_BITMAP_INTERPOLATION_MODE mode) {
  GetRenderTarget()->DrawBitmap(bitmap, dst_rect, opacity, mode, src_rect);
}

void Canvas::DrawLine(const Brush& brush,
                      const PointF& point1,
                      const PointF& point2,
                      float pen_width) {
  DCHECK(drawing());
  GetRenderTarget()->DrawLine(point1, point2, brush, pen_width);
}

void Canvas::DrawRectangle(const Brush& brush,
                           const RectF& rect,
                           float strokeWidth) {
  DCHECK(drawing());
  DCHECK(!rect.empty());
  GetRenderTarget()->DrawRectangle(rect, brush, strokeWidth);
}

void Canvas::DrawText(const TextFormat& text_format,
                      const Brush& brush,
                      const RectF& bounds,
                      const base::string16& text) {
  DCHECK(drawing());
  DCHECK(!bounds.empty());
  GetRenderTarget()->DrawText(text.data(), static_cast<uint32_t>(text.length()),
                              text_format, bounds, brush);
}

void Canvas::EndDraw() {
  DCHECK(drawing());
  DCHECK(GetRenderTarget());
  --batch_nesting_level_;
  if (batch_nesting_level_ == 0)
    CommitDraw();
  TRACE_EVENT_END1("gfx", "Canvas/Batch", "level", batch_nesting_level_);
}

void Canvas::FillRectangle(const Brush& brush, const RectF& rect) {
  DCHECK(drawing());
  DCHECK(rect);
  GetRenderTarget()->FillRectangle(rect, brush);
}

void Canvas::Flush() {
  DCHECK(drawing());
  auto const hr = GetRenderTarget()->Flush();
  if (SUCCEEDED(hr))
    return;

  if (hr == D2DERR_RECREATE_TARGET) {
    DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
    DidLostRenderTarget();
    return;
  }

  DVLOG(0) << "ID2D1RenderTarget::Flush: hr=" << std::hex << hr;
}

gfx::RectF Canvas::GetLocalBounds() const {
  return gfx::RectF(bounds_.size());
}

void Canvas::RemoveObserver(CanvasObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Canvas::RestoreScreenImage(const RectF& bounds) {
  DCHECK(!bounds_.empty());
  DCHECK(screen_bitmap_);
  auto const source_bounds =
      gfx::RectF(bounds.origin() + bounds_.origin(), bounds.size());
  DrawBitmap(*screen_bitmap_, bounds, source_bounds);
}

bool Canvas::SaveScreenImage(const RectF& rect_in) {
  DCHECK(!bounds_.empty());
  if (!screen_bitmap_)
    screen_bitmap_ = std::make_unique<Bitmap>(this);
  auto const rect =
      gfx::RectF(rect_in.origin() + bounds_.origin(), rect_in.size());
  auto const enclosing_rect = ToEnclosingRect(rect);
  const RectU source_rect(static_cast<uint32_t>(enclosing_rect.left()),
                          static_cast<uint32_t>(enclosing_rect.top()),
                          static_cast<uint32_t>(enclosing_rect.right()),
                          static_cast<uint32_t>(enclosing_rect.bottom()));
  const PointU dest_point(source_rect.origin());
  auto const hr =
      (*screen_bitmap_)
          ->CopyFromRenderTarget(&dest_point, GetRenderTarget(), &source_rect);
  if (FAILED(hr))
    DVLOG(0) << "ID2D1Bitmap->CopyFromRenderTarget hr=" << std::hex << hr;
  return SUCCEEDED(hr);
}

void Canvas::SetBounds(const RectF& new_bounds) {
  DCHECK(!bounds_.empty());
  DCHECK(!new_bounds.empty());
  DCHECK_EQ(new_bounds.left, 0.0f);
  DCHECK_EQ(new_bounds.top, 0.0f);
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  screen_bitmap_.reset();
  bitmap_id_ = ++global_bitmap_id;
  should_clear_ = true;
  DidChangeBounds(new_bounds);
}

void Canvas::SetInitialBounds(const RectF& bounds) {
  DCHECK(!bitmap_id_);
  DCHECK(bounds_.empty());
  bitmap_id_ = ++global_bitmap_id;
  bounds_ = bounds;
  AddDirtyRect(bounds_);
}

void Canvas::SetOffsetBounds(const gfx::RectF& bounds) {
  offset_ += bounds.origin();
  bounds_ = bounds_.Intersect(gfx::RectF(offset_, bounds.size()));
  DCHECK(!bounds_.empty());
  auto transform = D2D1::Matrix3x2F::Translation(offset_.x, offset_.y);
  GetRenderTarget()->SetTransform(transform);
}

bool Canvas::TryAddDirtyRect(const gfx::RectF& bounds) {
  auto const dirty_bounds = GetLocalBounds().Intersect(bounds);
  if (dirty_bounds.empty())
    return false;
  AddDirtyRect(dirty_bounds);
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// CanvasForHwnd
//
CanvasForHwnd::CanvasForHwnd(HWND hwnd)
    : hwnd_(hwnd), swap_chain_(SwapChain::CreateForHwnd(hwnd)) {
  SetInitialBounds(swap_chain_->bounds());
}

CanvasForHwnd::~CanvasForHwnd() {}

// Canvas
void CanvasForHwnd::AddDirtyRectImpl(const RectF& new_dirty_rect) {
  swap_chain_->AddDirtyRect(new_dirty_rect);
}

void CanvasForHwnd::DidCallEndDraw() {
  swap_chain_->Present();
}

void CanvasForHwnd::DidChangeBounds(const RectF& new_bounds) {
  swap_chain_->DidChangeBounds(new_bounds);
}

void CanvasForHwnd::DidLostRenderTarget() {
  swap_chain_.reset(SwapChain::CreateForHwnd(hwnd_));
  DidCreateRenderTarget();
}

ID2D1RenderTarget* CanvasForHwnd::GetRenderTarget() const {
  return swap_chain_->d2d_device_context();
}

}  // namespace gfx
