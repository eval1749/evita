// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/canvas.h"

#include <d2d1_2helper.h>

#include <cmath>
#include <utility>

#include "evita/gfx/bitmap.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/swap_chain.h"
#include "evita/gfx/text_format.h"

#pragma comment(lib, "d3d11.lib")

#define DEBUG_DRAW 0

std::ostream& operator<<(std::ostream& ostream,
                         D2D1_TEXT_ANTIALIAS_MODE mode) {
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

//////////////////////////////////////////////////////////////////////
//
// Canvas::AxisAlignedClipScope
//
Canvas::AxisAlignedClipScope::AxisAlignedClipScope(
    Canvas* canvas, const RectF& bounds, D2D1_ANTIALIAS_MODE alias_mode)
    : canvas_(canvas) {
  DCHECK(!bounds.empty());
  (*canvas_)->PushAxisAlignedClip(bounds, alias_mode);
}

Canvas::AxisAlignedClipScope::AxisAlignedClipScope(Canvas* canvas,
                                                   const RectF& bounds)
    : AxisAlignedClipScope(canvas, bounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE) {
}

Canvas::AxisAlignedClipScope::~AxisAlignedClipScope() {
  (*canvas_)->PopAxisAlignedClip();
}

//////////////////////////////////////////////////////////////////////
//
// Canvas::DrawingScope
//
Canvas::DrawingScope::DrawingScope(Canvas* canvas)
    : canvas_(canvas) {
  canvas_->BeginDraw();
}

Canvas::DrawingScope::~DrawingScope() {
  // TODO(yosi) Should DrawingScope take mutable Canvas?
  canvas_->EndDraw();
}

//////////////////////////////////////////////////////////////////////
//
// Canvas::Observer
//
Canvas::Observer::Observer() {
}

Canvas::Observer::~Observer() {
}

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
Canvas::Canvas()
    : batch_nesting_level_(0), factory_set_(FactorySet::instance()),
      work_(nullptr) {
}

Canvas::~Canvas() {
}

void Canvas::set_dirty_rect(const RectF& new_dirty_rect) {
  AddDirtyRect(new_dirty_rect);
}

void Canvas::AddDirtyRect(const RectF&) {
}

void Canvas::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void Canvas::BeginDraw() {
  DCHECK(!bounds_.empty());
  DCHECK(GetRenderTarget());
  if (!batch_nesting_level_)
    GetRenderTarget()->BeginDraw();
  ++batch_nesting_level_;
}

void Canvas::Clear(const ColorF& color) {
  GetRenderTarget()->Clear(color);
}

void Canvas::DidCallEndDraw() {
}

void Canvas::DidCreateRenderTarget() {
  SizeF dpi;
  GetRenderTarget()->GetDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
  FOR_EACH_OBSERVER(Observer, observers_, ShouldDiscardResources());
}

void Canvas::DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                        const RectF& src_rect, float opacity,
                        D2D1_BITMAP_INTERPOLATION_MODE mode) {
  GetRenderTarget()->DrawBitmap(bitmap, dst_rect, opacity, mode, src_rect);
}

void Canvas::DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                      float strokeWidth) {
  DCHECK(drawing());
  GetRenderTarget()->DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                              strokeWidth);
}

void Canvas::DrawLine(const Brush& brush, float sx, float sy,
                      float ex, float ey, float strokeWidth) {
  DCHECK(drawing());
  GetRenderTarget()->DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                              strokeWidth);
}

void Canvas::DrawRectangle(const Brush& brush, const RECT& rc,
                           float strokeWidth) {
  DrawRectangle(brush, RectF(rc), strokeWidth);
}

void Canvas::DrawRectangle(const Brush& brush, const RectF& rect,
                           float strokeWidth) {
  DCHECK(drawing());
  DCHECK(rect);
  GetRenderTarget()->DrawRectangle(rect, brush, strokeWidth);
}

void Canvas::DrawText(const TextFormat& text_format,
                      const Brush& brush,
                      const RECT& rc,
                      const char16* pwch, size_t cwch) {
  DCHECK(drawing());
  auto rect = RectF(rc);
  DCHECK(rect);
  GetRenderTarget()->DrawText(pwch, static_cast<uint32_t>(cwch), text_format,
                              rect, brush);
}

bool Canvas::EndDraw() {
  DCHECK(drawing());
  DCHECK(GetRenderTarget());
  --batch_nesting_level_;
  auto const hr = batch_nesting_level_ ? GetRenderTarget()->Flush() :
                                         GetRenderTarget()->EndDraw();
  if (SUCCEEDED(hr)) {
    if (!batch_nesting_level_)
      DidCallEndDraw();
    return true;
  }
  if (hr == D2DERR_RECREATE_TARGET) {
    DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
    DidLostRenderTarget();
  } else {
    DVLOG(0) << "ID2D1RenderTarget::Flush: hr=" << std::hex << hr;
  }
  return false;
}

void Canvas::FillRectangle(const Brush& brush, int left, int top,
                              int right, int bottom) {
  GetRenderTarget()->FillRectangle(RectF(left, top, right, bottom), brush);
}

void Canvas::FillRectangle(const Brush& brush, float left, float top,
                              float right, float bottom) {
  FillRectangle(brush, RectF(left, top, right, bottom));
}

void Canvas::FillRectangle(const Brush& brush, const RECT& rc) {
  FillRectangle(brush, RectF(rc));
}

void Canvas::FillRectangle(const Brush& brush, const RectF& rect) {
  DCHECK(drawing());
  DCHECK(rect);
  GetRenderTarget()->FillRectangle(rect, brush);
}

void Canvas::Flush() {
  DCHECK(drawing());
  D2D1_TAG tag1, tag2;
  auto const hr = GetRenderTarget()->Flush(&tag1, &tag2);
  if (SUCCEEDED(hr))
    return;
  if (hr == D2DERR_RECREATE_TARGET)
    DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
  else
    DVLOG(0) << "ID2D1RenderTarget::Flush: hr=" << std::hex << hr;
  DidLostRenderTarget();
}

void Canvas::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

bool Canvas::SaveScreenImage(const RectF& rect) {
  DCHECK(!bounds_.empty());
  if (!screen_bitmap_)
    screen_bitmap_ = std::make_unique<Bitmap>(this);
  auto const enclosing_rect = ToEnclosingRect(rect);
  const RectU source_rect(static_cast<uint32_t>(enclosing_rect.left),
                          static_cast<uint32_t>(enclosing_rect.top),
                          static_cast<uint32_t>(enclosing_rect.right),
                          static_cast<uint32_t>(enclosing_rect.bottom));
  const PointU dest_point(source_rect.origin());
  auto const hr = (*screen_bitmap_)->CopyFromRenderTarget(&dest_point,
      GetRenderTarget(), &source_rect);
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
  DidChangeBounds(new_bounds);
}

void Canvas::SetInitialBounds(const RectF& bounds) {
  DCHECK(bounds_.empty());
  bounds_ = bounds;
}

//////////////////////////////////////////////////////////////////////
//
// CanvasForHwnd
//
CanvasForHwnd::CanvasForHwnd(HWND hwnd)
    : hwnd_(hwnd), swap_chain_(SwapChain::CreateForHwnd(hwnd)) {
  SetInitialBounds(swap_chain_->bounds());
}

CanvasForHwnd::~CanvasForHwnd() {
}

// Canvas
void CanvasForHwnd::AddDirtyRect(const RectF& new_dirty_rect) {
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

//////////////////////////////////////////////////////////////////////
//
// LegacyCanvasForHwnd
//
LegacyCanvasForHwnd::LegacyCanvasForHwnd(HWND hwnd) : hwnd_(hwnd) {
  AttachRenderTarget();
  auto const size = hwnd_render_target_->GetPixelSize();
  SetInitialBounds(gfx::RectF(gfx::SizeF(
      static_cast<float>(size.width), static_cast<float>(size.height))));
}

LegacyCanvasForHwnd::~LegacyCanvasForHwnd() {
}

void LegacyCanvasForHwnd::AttachRenderTarget() {
  RECT rc;
  ::GetClientRect(hwnd_, &rc);
  auto const pixel_format = D2D1::PixelFormat(
      DXGI_FORMAT_B8G8R8A8_UNORM,
      D2D1_ALPHA_MODE_PREMULTIPLIED);
  auto const size = SizeU(rc.right - rc.left, rc.bottom - rc.top);
  auto const usage = D2D1_RENDER_TARGET_USAGE_NONE;
  common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
  COM_VERIFY(FactorySet::d2d1().CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                   pixel_format, 0.0f, 0.0f,
                                   usage),
      D2D1::HwndRenderTargetProperties(hwnd_, size,
                                       D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
      &hwnd_render_target_));
  DidCreateRenderTarget();
}

// Canvas
void LegacyCanvasForHwnd::DidChangeBounds(const RectF& new_bounds) {
  auto const enclosing_rect = ToEnclosingRect(new_bounds);
  COM_VERIFY(hwnd_render_target_->Resize(D2D1::SizeU(
      static_cast<uint32_t>(enclosing_rect.width()),
      static_cast<uint32_t>(enclosing_rect.height()))));
}

void LegacyCanvasForHwnd::DidLostRenderTarget() {
  hwnd_render_target_.release();
  AttachRenderTarget();
}

ID2D1RenderTarget* LegacyCanvasForHwnd::GetRenderTarget() const {
  return hwnd_render_target_;
}

}  // namespace gfx
