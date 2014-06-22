// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/canvas.h"

#include <d2d1_2helper.h>
#pragma warning(push)
#pragma warning(disable: 4061 4365 4917)
#include <d3d11_1.h>
#pragma warning(pop)
#include <VersionHelpers.h>
#include <cmath>
#include <utility>

#include "common/win/rect_ostream.h"
#include "evita/gfx/bitmap.h"
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
    const Canvas& canvas, const RectF& bounds, D2D1_ANTIALIAS_MODE alias_mode)
    : canvas_(canvas) {
  canvas_->PushAxisAlignedClip(bounds, alias_mode);
}

Canvas::AxisAlignedClipScope::AxisAlignedClipScope(const Canvas& canvas,
                                                   const RectF& bounds)
    : AxisAlignedClipScope(canvas, bounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE) {
}

Canvas::AxisAlignedClipScope::~AxisAlignedClipScope() {
  canvas_->PopAxisAlignedClip();
}

//////////////////////////////////////////////////////////////////////
//
// Canvas::DrawingScope
//
Canvas::DrawingScope::DrawingScope(const Canvas& gfx)
    : gfx_(const_cast<Canvas*>(&gfx)) {
  gfx_->BeginDraw();
}

Canvas::DrawingScope::~DrawingScope() {
  // TODO(yosi) Should DrawingScope take mutable Canvas?
  gfx_->EndDraw();
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
Canvas::Canvas(DwmSupport dwm_support)
    : batch_nesting_level_(0),
      dwm_support_(dwm_support),
      factory_set_(FactorySet::instance()),
      hwnd_(nullptr),
      work_(nullptr) {
}

Canvas::Canvas() : Canvas(DwmSupport::NotSupportDwm) {
}

Canvas::~Canvas() {
}

void Canvas::set_dirty_rect(const Rect& new_dirty_rect) const {
  DCHECK(!new_dirty_rect.empty());
  if (dirty_rects_.empty()) {
    dirty_rects_.push_back(new_dirty_rect);
    return;
  }
  for (const auto& dirty_rect : dirty_rects_) {
    if (dirty_rect.Contains(new_dirty_rect))
        return;
  }
  std::vector<Rect> new_dirty_rects;
  for (const auto& dirty_rect : dirty_rects_) {
    if (!new_dirty_rect.Contains(dirty_rect))
      new_dirty_rects.push_back(dirty_rect);
  }
  new_dirty_rects.push_back(new_dirty_rect);
  dirty_rects_ = new_dirty_rects;
}

void Canvas::set_dirty_rect(const RectF& new_dirty_rect) const {
  set_dirty_rect(Rect(static_cast<int>(::floor(new_dirty_rect.left)),
                      static_cast<int>(::floor(new_dirty_rect.top)),
                      static_cast<int>(::ceil(new_dirty_rect.right)),
                      static_cast<int>(::ceil(new_dirty_rect.bottom))));
}

ID2D1RenderTarget& Canvas::render_target() const {
  DCHECK(render_target_) << "No ID2D1RenderTarget";
  return *render_target_.get();
}

void Canvas::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void Canvas::BeginDraw() const {
  ASSERT(render_target_);
  if (!batch_nesting_level_)
    render_target_->BeginDraw();
  ++batch_nesting_level_;
}

void Canvas::DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity,
                          D2D1_BITMAP_INTERPOLATION_MODE mode) const {
  render_target_->DrawBitmap(bitmap, dst_rect, opacity, mode, src_rect);
}

void Canvas::DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth) const {
  DCHECK(drawing());
  render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                           strokeWidth);
}

void Canvas::DrawLine(const Brush& brush, float sx, float sy,
                        float ex, float ey, float strokeWidth) const {
  DCHECK(drawing());
  render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                           strokeWidth);
}

void Canvas::DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth) const {
  DrawRectangle(brush, RectF(rc), strokeWidth);
}

void Canvas::DrawRectangle(const Brush& brush, const RectF& rect,
                            float strokeWidth) const {
  DCHECK(drawing());
  DCHECK(rect);
  render_target().DrawRectangle(rect, brush, strokeWidth);
}

void Canvas::DrawText(const TextFormat& text_format,
                      const Brush& brush,
                      const RECT& rc,
                      const char16* pwch, size_t cwch) const {
  DCHECK(drawing());
  auto rect = RectF(rc);
  DCHECK(rect);
  render_target().DrawText(pwch, static_cast<uint32_t>(cwch), text_format,
                           rect, brush);
}

bool Canvas::EndDraw() {
  ASSERT(drawing());
  ASSERT(render_target_);
  --batch_nesting_level_;
  if (batch_nesting_level_) {
    auto const hr = render_target_->Flush();
    if (SUCCEEDED(hr))
      return true;
    if (hr == D2DERR_RECREATE_TARGET) {
      DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
    } else {
      DVLOG(0) << "ID2D1RenderTarget::Flush: hr=" << std::hex << hr;
    }
  } else {
      auto const hr = render_target_->EndDraw();
      if (SUCCEEDED(hr)) {
        if (dwm_support_ == DwmSupport::SupportDwm) {
            dirty_rects_.clear();
            return true;
          }
          if (dirty_rects_.empty()) {
            DVLOG(0) << "Canvas::EndDraw: no dirty";
          } else {
            #if DEBUG_DRAW
              DVLOG(0) << "Canvas::EndDraw: swap #dirty=" <<
                  dirty_rects_.size() << " " << dirty_rects_[0];
            #endif
            DXGI_PRESENT_PARAMETERS parameters = {0};
            parameters.DirtyRectsCount = dirty_rects_.size();
            parameters.pDirtyRects = dirty_rects_.data();
            parameters.pScrollRect = nullptr;
            parameters.pScrollOffset = nullptr;
            COM_VERIFY(dxgi_swap_chain_->Present1(1, 0, &parameters));
            dirty_rects_.clear();
          }
        return true;
      }
      if (hr == D2DERR_RECREATE_TARGET) {
        DVLOG(0) << "Canvas::End D2DERR_RECREATE_TARGET";
      } else {
        DVLOG(0) << "ID2D1RenderTarget::EndDraw: hr=" << std::hex << hr;
      }
  }
  dxgi_swap_chain_.reset();
  render_target_.reset();
  const_cast<Canvas*>(this)->Reinitialize();
  return false;
}

void Canvas::FillRectangle(const Brush& brush, int left, int top,
                              int right, int bottom) const {
  render_target().FillRectangle(RectF(left, top, right, bottom), brush);
}

void Canvas::FillRectangle(const Brush& brush, float left, float top,
                              float right, float bottom) const {
  FillRectangle(brush, RectF(left, top, right, bottom));
}

void Canvas::FillRectangle(const Brush& brush, const RECT& rc) const {
  FillRectangle(brush, RectF(rc));
}

void Canvas::FillRectangle(const Brush& brush, const RectF& rect) const {
  DCHECK(drawing());
  DCHECK(rect);
  render_target().FillRectangle(rect, brush);
}

void Canvas::Flush() const {
  ASSERT(drawing());
  D2D1_TAG tag1, tag2;
  COM_VERIFY(render_target_->Flush(&tag1, &tag2));
  ASSERT(!tag1 && !tag2);
}

void Canvas::Init(HWND hwnd) {
  ASSERT(!hwnd_);
  hwnd_ = hwnd;
  Reinitialize();
}

void Canvas::Reinitialize() {
  DCHECK(!render_target_);
  DCHECK(hwnd_);

  if (dwm_support_ == DwmSupport::SupportDwm){
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
    return;
  }

  static bool did_check_windows_version;
  static bool is_windows_8;
  if (!did_check_windows_version) {
    is_windows_8 = ::IsWindows8OrGreater();
  }

  uint32_t creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                            D3D11_CREATE_DEVICE_SINGLETHREADED;
  #if _DEBUG
    creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
  #endif
  D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1,
  };

  common::ComPtr<ID3D11Device> device;
  common::ComPtr<ID3D11DeviceContext> device_context;
  D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_9_1;
  COM_VERIFY(::D3D11CreateDevice(
      nullptr, // default adopter
      D3D_DRIVER_TYPE_HARDWARE,
      0,
      creation_flags,
      feature_levels, arraysize(feature_levels),
      D3D11_SDK_VERSION,
      &device,
      &feature_level,
      &device_context));

  common::ComPtr<IDXGIDevice1> dxgi_device;
  COM_VERIFY(dxgi_device.QueryFrom(device));

  common::ComPtr<ID2D1Device> d2d_device;
  COM_VERIFY(FactorySet::d2d1().CreateDevice(dxgi_device, &d2d_device));

  common::ComPtr<ID2D1DeviceContext> d2d_device_context;
  COM_VERIFY(d2d_device->CreateDeviceContext(
      D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context));

 // Allocate a descriptor.
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.Width = 0;  // use automatic sizing
  swap_chain_desc.Height = 0;
  // this is the most common swap chain format
  swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swap_chain_desc.Stereo = false;
  swap_chain_desc.SampleDesc.Count = 1; // don't use multi-sampling
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;  // use double buffering to enable flip
  if (is_windows_8)
    swap_chain_desc.Scaling = DXGI_SCALING_NONE;
  else
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
  // Using DXGI_SWAP_EFFECT_SEQUENTIAL or DXGI_SWAP_EFFECT_DISCARD cause
  // |CreateSwapChainForHwnd| returns 0X887A0001.
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swap_chain_desc.Flags = 0;

  common::ComPtr<IDXGIAdapter> dxgi_adapter;
  dxgi_device->GetAdapter(&dxgi_adapter);

  common::ComPtr<IDXGIFactory2> dxgi_factory;
  dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));

  COM_VERIFY(dxgi_factory->CreateSwapChainForHwnd(
      device, hwnd_, &swap_chain_desc, nullptr, nullptr, &dxgi_swap_chain_));

  {
    DXGI_RGBA color;
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;
    COM_VERIFY(dxgi_swap_chain_->SetBackgroundColor(&color));
  }

  // Ensure that DXGI doesn't queue more than one frame at a time.
  COM_VERIFY(dxgi_device->SetMaximumFrameLatency(1));
  SetupRenderTarget(d2d_device_context);
  render_target_.reset(d2d_device_context);
  SizeF dpi;
  render_target_->GetDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
  FOR_EACH_OBSERVER(Observer, observers_, ShouldDiscardResources());
}

void Canvas::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void Canvas::Resize(const Rect& rect) const {
  DCHECK(!rect.empty());

  if (dwm_support_ == DwmSupport::SupportDwm){
    SizeU size(rect.width(), rect.height());
    common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
    COM_VERIFY(hwnd_render_target.QueryFrom(render_target_));
    COM_VERIFY(hwnd_render_target->Resize(size));
    return;
  }

  screen_bitmap_.reset();
  target_bounds_ = rect;

  common::ComPtr<ID2D1DeviceContext> d2d_device_context;
  COM_VERIFY(d2d_device_context.QueryFrom(render_target_));

  // Release resources in swap chain.
  d2d_device_context->SetTarget(nullptr);

  COM_VERIFY(dxgi_swap_chain_->ResizeBuffers(0u,
      static_cast<uint32_t>(rect.width()),
      static_cast<uint32_t>(rect.height()), DXGI_FORMAT_UNKNOWN, 0u));
  SetupRenderTarget(d2d_device_context);
}

bool Canvas::SaveScreenImage(const RectF& rect) const {
  if (!screen_bitmap_)
    screen_bitmap_ = std::make_unique<Bitmap>(*this);
  const RectU source_rect(static_cast<uint32_t>(::floor(rect.left)),
                          static_cast<uint32_t>(::floor(rect.top)),
                          static_cast<uint32_t>(::ceil(rect.right)),
                          static_cast<uint32_t>(::ceil(rect.bottom)));
  const PointU dest_point(source_rect.left_top());
  auto const hr = (*screen_bitmap_)->CopyFromRenderTarget(&dest_point,
      render_target_, &source_rect);
  if (FAILED(hr))
    DVLOG(0) << "ID2D1Bitmap->CopyFromRenderTarget hr=" << std::hex << hr;
  return SUCCEEDED(hr);
}

void Canvas::SetupRenderTarget(ID2D1DeviceContext* d2d_device_context) const {
  common::ComPtr<IDXGISurface> dxgi_back_buffer;
  dxgi_swap_chain_->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer));

  D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
      D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
      FactorySet::instance()->pixels_per_dip().width,
      FactorySet::instance()->pixels_per_dip().height);
  common::ComPtr<ID2D1Bitmap1> d2d_back_buffer;
  COM_VERIFY(d2d_device_context->CreateBitmapFromDxgiSurface(
      dxgi_back_buffer, &bitmapProperties, &d2d_back_buffer));
  d2d_device_context->SetTarget(d2d_back_buffer);
  d2d_device_context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
}

}  // namespace gfx
