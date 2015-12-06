// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <d2d1_2helper.h>
#include <dxgi1_3.h>

#include "evita/gfx/swap_chain.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/dx_device.h"
#include "evita/gfx/rect_conversions.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// SwapChain
//
SwapChain::SwapChain(ID2D1Device* d2d_device,
                     common::ComPtr<IDXGISwapChain2> swap_chain)
    : is_first_present_(true),
      is_ready_(false),
      swap_chain_(swap_chain),
      swap_chain_waitable_(swap_chain_->GetFrameLatencyWaitableObject()) {
  // Antialias Mode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
  // Primitive Blend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER
  // Text Antialias Mode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT
  COM_VERIFY(d2d_device->CreateDeviceContext(
      D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context_));
  UpdateDeviceContext();
}

SwapChain::~SwapChain() {
  d2d_device_context_->SetTarget(nullptr);
  d2d_device_context_.reset();
}

void SwapChain::AddDirtyRect(const RectF& new_dirty_rect_f) {
  DCHECK(bounds_.Contains(new_dirty_rect_f));
  auto const new_dirty_rect = ToEnclosingRect(new_dirty_rect_f);
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

SwapChain* SwapChain::CreateForComposition(const RectF& bounds) {
  DCHECK(!bounds.empty());
  TRACE_EVENT0("gfx", "SwapChain::CreateForComposition");
  // TODO(eval1749): We should use ToEnclosedRect().
  const auto size = gfx::SizeU(static_cast<uint32_t>(bounds.width()),
                               static_cast<uint32_t>(bounds.height()));

  auto const device = DxDevice::instance();

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
  swap_chain_desc.Width = size.width;
  swap_chain_desc.Height = size.height;
  swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1;  // don't use multi-sampling
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;  // use double buffering to enable flip
  swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  common::ComPtr<IDXGISwapChain1> swap_chain1;
  COM_VERIFY(device->dxgi_factory()->CreateSwapChainForComposition(
      device->dxgi_device(), &swap_chain_desc, nullptr, &swap_chain1));
  common::ComPtr<IDXGISwapChain2> swap_chain2;
  COM_VERIFY(swap_chain2.QueryFrom(swap_chain1));

  // http://msdn.microsoft.com/en-us/library/windows/apps/dn448914.aspx
  // Swapchains created with the
  // DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT flag use their own
  // per-swapchain latency setting instead of the one associated with the DXGI
  // device. The default per-swapchain latency is 1, which ensures that DXGI
  // does not queue more than one frame at a time. This both reduces latency
  // and ensures that the application will only render after each VSync,
  // minimizing power consumption.
  COM_VERIFY(swap_chain2->SetMaximumFrameLatency(1));
  return new SwapChain(device->d2d_device(), swap_chain2);
}

SwapChain* SwapChain::CreateForHwnd(HWND hwnd) {
  DCHECK(hwnd);

  auto const device = DxDevice::instance();

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.Width = 0;  // use automatic sizing
  swap_chain_desc.Height = 0;
  // this is the most common swap chain format
  swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swap_chain_desc.Stereo = false;
  swap_chain_desc.SampleDesc.Count = 1;  // don't use multi-sampling
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;  // use double buffering to enable flip
  swap_chain_desc.Scaling = DXGI_SCALING_NONE;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  common::ComPtr<IDXGISwapChain1> swap_chain1;
  COM_VERIFY(device->dxgi_factory()->CreateSwapChainForHwnd(
      device->dxgi_device(), hwnd, &swap_chain_desc, nullptr, nullptr,
      &swap_chain1));

  common::ComPtr<IDXGISwapChain2> swap_chain2;
  COM_VERIFY(swap_chain2.QueryFrom(swap_chain1));
  return new SwapChain(device->d2d_device(), swap_chain2);
}

void SwapChain::DidChangeBounds(const RectF& new_bounds) {
  TRACE_EVENT0("gfx", "SwapChain::DidChangeBounds");
  d2d_device_context_->SetTarget(nullptr);
  bounds_ = new_bounds;
  auto const enclosing_rect = ToEnclosingRect(new_bounds);
  COM_VERIFY(swap_chain_->ResizeBuffers(
      0u, static_cast<UINT>(enclosing_rect.width()),
      static_cast<UINT>(enclosing_rect.height()), DXGI_FORMAT_UNKNOWN,
      DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));
  UpdateDeviceContext();
}

bool SwapChain::IsReady() {
  if (is_ready_)
    return true;
  auto const wait = ::WaitForSingleObject(swap_chain_waitable_, 0);
  switch (wait) {
    case WAIT_OBJECT_0:
      is_ready_ = true;
      return true;
    case WAIT_TIMEOUT:
      return false;
    default:
      NOTREACHED();
  }
  return false;
}

void SwapChain::Present() {
  if (dirty_rects_.empty())
    return;
  DCHECK(is_ready_);
  TRACE_EVENT1("gfx", "SwapChain::Present", "count", dirty_rects_.size());
  DXGI_PRESENT_PARAMETERS parameters = {0};
  std::vector<RECT> dirty_rects;
  if (!is_first_present_) {
    for (auto const rect : dirty_rects_) {
      dirty_rects.push_back(static_cast<RECT>(rect));
    }
    parameters.DirtyRectsCount = static_cast<UINT>(dirty_rects.size());
    parameters.pDirtyRects = dirty_rects.data();
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;
  }
  auto const flags = DXGI_PRESENT_DO_NOT_WAIT | DXGI_PRESENT_RESTART;
  COM_VERIFY(swap_chain_->Present1(0, flags, &parameters));
  dirty_rects_.clear();
  is_ready_ = false;
  is_first_present_ = false;
}

void SwapChain::UpdateDeviceContext() {
  TRACE_EVENT0("gfx", "SwapChain::UpdateDeviceContext");
  {
    DXGI_RGBA color;
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;
    COM_VERIFY(swap_chain_->SetBackgroundColor(&color));
  }

  // Allocate back buffer for d2d device context.
  {
    common::ComPtr<IDXGISurface> dxgi_back_buffer;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer));

    common::ComPtr<ID2D1Factory> d2d_factory;
    d2d_device_context_->GetFactory(&d2d_factory);

    float dpi_x, dpi_y;
    d2d_factory->GetDesktopDpi(&dpi_x, &dpi_y);
    auto const bitmap_properties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                          D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpi_x, dpi_y);

    common::ComPtr<ID2D1Bitmap1> d2d_back_buffer;
    COM_VERIFY(d2d_device_context_->CreateBitmapFromDxgiSurface(
        dxgi_back_buffer, bitmap_properties, &d2d_back_buffer));
    d2d_device_context_->SetTarget(d2d_back_buffer);

    auto const size = d2d_back_buffer->GetPixelSize();
    bounds_ = gfx::RectF(gfx::SizeF(static_cast<float>(size.width),
                                    static_cast<float>(size.height)));
  }

  d2d_device_context_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
  is_first_present_ = true;
}

}  // namespace gfx
