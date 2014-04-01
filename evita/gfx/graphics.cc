// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/graphics.h"

#include <utility>

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
// Graphics
//
Graphics::Graphics(ID2D1RenderTarget* render_target)
    : batch_nesting_level_(0),
      factory_set_(FactorySet::instance()),
      hwnd_(nullptr),
      render_target_(render_target),
      work_(nullptr) {
  if (render_target_) {
    SizeF dpi;
    render_target_->GetDpi(&dpi.width, &dpi.height);
    UpdateDpi(dpi);
  }
}

Graphics::Graphics(Graphics&& other)
    : batch_nesting_level_(0),
      factory_set_(std::move(other.factory_set_)),
      hwnd_(other.hwnd_),
      render_target_(std::move(other.render_target_)),
      work_(nullptr) {
  other.hwnd_ = nullptr;
}

Graphics::Graphics() : Graphics(nullptr) {
}

Graphics::~Graphics() {
}

Graphics& Graphics::operator=(Graphics&& other) {
  factory_set_ = std::move(other.factory_set_);
  render_target_ = std::move(other.render_target_);
  hwnd_ = other.hwnd_;
  other.hwnd_ = nullptr;
  return *this;
}

ID2D1RenderTarget& Graphics::render_target() const {
  DCHECK(render_target_) << "No ID2D1RenderTarget";
  return *render_target_.get();
}

void Graphics::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void Graphics::BeginDraw() const {
  #if DEBUG_DRAW
    DEBUG_PRINTF("%p nesting=%d\n", render_target_, batch_nesting_level_);
  #endif
  ASSERT(render_target_);
  if (!batch_nesting_level_)
    render_target_->BeginDraw();
  ++batch_nesting_level_;
}

Graphics Graphics::CreateCompatible(const SizeF& size) const {
  common::ComPtr<ID2D1BitmapRenderTarget> compatible_target;
  auto const hr = render_target_->CreateCompatibleRenderTarget(
    size, &compatible_target);
  if (FAILED(hr)) {
    DVLOG(0) << "CreateCompatibleRenderTarget: hr=0x" << std::hex << hr;
    return Graphics();
  }
  common::ComPtr<IDWriteRenderingParams> params;
  render_target_->GetTextRenderingParams(&params);
  if (params) {
    compatible_target->SetTextRenderingParams(params);
    DVLOG(0) << "ClearTypeLevel=" << params->GetClearTypeLevel() <<
                " Gamma=" << params->GetGamma() <<
                " PixelGeometry=" << params->GetPixelGeometry() <<
                " RenderingMode=" << params->GetRenderingMode();
  }
  DVLOG(0) << "Before AntialiasMode=" <<
    compatible_target->GetTextAntialiasMode();
  auto const antialias_mode = render_target_->GetTextAntialiasMode();
  compatible_target->SetTextAntialiasMode(antialias_mode);
  DVLOG(0) << "AntialiasMode=" << antialias_mode;
  return std::move(Graphics(compatible_target.release()));
}

void Graphics::DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity,
                          D2D1_BITMAP_INTERPOLATION_MODE mode) const {
  render_target_->DrawBitmap(bitmap, dst_rect, opacity, mode, src_rect);
}

void Graphics::DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth) const {
  DCHECK(drawing());
  render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                           strokeWidth);
}

void Graphics::DrawLine(const Brush& brush, float sx, float sy,
                        float ex, float ey, float strokeWidth) const {
  DCHECK(drawing());
  render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                           strokeWidth);
}

void Graphics::DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth) const {
  DrawRectangle(brush, RectF(rc), strokeWidth);
}

void Graphics::DrawRectangle(const Brush& brush, const RectF& rect,
                            float strokeWidth) const {
  DCHECK(drawing());
  DCHECK(rect);
  render_target().DrawRectangle(rect, brush, strokeWidth);
}

void Graphics::DrawText(const TextFormat& text_format,
                      const Brush& brush,
                      const RECT& rc,
                      const char16* pwch, size_t cwch) const {
  DCHECK(drawing());
  auto rect = RectF(rc);
  DCHECK(rect);
  render_target().DrawText(pwch, static_cast<uint32_t>(cwch), text_format,
                           rect, brush);
}

bool Graphics::EndDraw() {
  #if DEBUG_DRAW
    DEBUG_PRINTF("%p nesting=%d\n", render_target_, batch_nesting_level_);
  #endif
  ASSERT(drawing());
  ASSERT(render_target_);
  --batch_nesting_level_;
  if (batch_nesting_level_) {
    auto const hr = render_target_->Flush();
    if (SUCCEEDED(hr))
      return true;
    if (hr == D2DERR_RECREATE_TARGET) {
      Debugger::Printf("Got D2DERR_RECREATE_TARGET\n", hr);
    } else {
      Debugger::Printf("ID2D1RenderTarget::Flush failed hr=0x%0X\n", hr);
    }
  } else {
      auto const hr = render_target_->EndDraw();
      if (SUCCEEDED(hr))
        return true;
      if (hr == D2DERR_RECREATE_TARGET) {
        Debugger::Printf("Got D2DERR_RECREATE_TARGET\n", hr);
      } else {
        Debugger::Printf("ID2D1RenderTarget::EndDraw failed hr=0x%0X\n", hr);
      }
  }
  const_cast<Graphics*>(this)->render_target_.reset();
  const_cast<Graphics*>(this)->Reinitialize();
  return false;
}

void Graphics:: FillRectangle(const Brush& brush, int left, int top,
                              int right, int bottom) const {
  render_target().FillRectangle(RectF(left, top, right, bottom), brush);
}

void Graphics:: FillRectangle(const Brush& brush, float left, float top,
                              float right, float bottom) const {
  FillRectangle(brush, RectF(left, top, right, bottom));
}

void Graphics:: FillRectangle(const Brush& brush, const RECT& rc) const {
  FillRectangle(brush, RectF(rc));
}

void Graphics:: FillRectangle(const Brush& brush, const RectF& rect) const {
  DCHECK(drawing());
  DCHECK(rect);
  render_target().FillRectangle(rect, brush);
}

void Graphics::Flush() const {
  ASSERT(drawing());
  D2D1_TAG tag1, tag2;
  COM_VERIFY(render_target_->Flush(&tag1, &tag2));
  ASSERT(!tag1 && !tag2);
}

void Graphics::Init(HWND hwnd) {
  ASSERT(!hwnd_);
  hwnd_ = hwnd;
  Reinitialize();
}

void Graphics::Reinitialize() {
  DCHECK(!render_target_);
  DCHECK(hwnd_);
  RECT rc;
  ::GetClientRect(hwnd_, &rc);
  auto const pixel_format = D2D1::PixelFormat(
      DXGI_FORMAT_B8G8R8A8_UNORM,
      D2D1_ALPHA_MODE_PREMULTIPLIED);
  auto const size = SizeU(rc.right - rc.left, rc.bottom - rc.top);
  // TODO: When should use D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE?
  //auto const usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
  auto const usage = D2D1_RENDER_TARGET_USAGE_NONE;
  common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
  COM_VERIFY(FactorySet::d2d1().CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                   pixel_format, 0.0f, 0.0f,
                                   usage),
      D2D1::HwndRenderTargetProperties(hwnd_, size,
                                       D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
      &hwnd_render_target));
  render_target_.reset(hwnd_render_target);
  SizeF dpi;
  render_target_->GetDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
  render_target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
  FOR_EACH_OBSERVER(Observer, observers_, ShouldDiscardResources());
}

void Graphics::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void Graphics::Resize(const Rect& rc) const {
  SizeU size(rc.width(), rc.height());
  common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
  COM_VERIFY(hwnd_render_target.QueryFrom(render_target_));
  COM_VERIFY(hwnd_render_target->Resize(size));
}

//////////////////////////////////////////////////////////////////////
//
// Graphics::AxisAlignedClipScope
//
Graphics::AxisAlignedClipScope::AxisAlignedClipScope(
    const Graphics& gfx, const RectF& rect)
    : gfx_(gfx) {
  gfx_->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

Graphics::AxisAlignedClipScope::~AxisAlignedClipScope() {
  gfx_->PopAxisAlignedClip();
}

//////////////////////////////////////////////////////////////////////
//
// Graphics::DrawingScope
//
Graphics::DrawingScope::DrawingScope(const Graphics& gfx) : gfx_(gfx) {
  gfx_.BeginDraw();
}

Graphics::DrawingScope::~DrawingScope() {
  // TODO(yosi) Should DrawingScope take mutable Graphics?
  const_cast<Graphics&>(gfx_).EndDraw();
}

//////////////////////////////////////////////////////////////////////
//
// Graphics::Observer
//
Graphics::Observer::Observer() {
}

Graphics::Observer::~Observer() {
}

}  // namespace gfx
