// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/swap_chain.h"

#include <d2d1_2helper.h>
#pragma warning(push)
#pragma warning(disable: 4061 4365 4917)
#include <d3d11_1.h>
#pragma warning(pop)
#include <dxgi1_3.h>

#include "base/logging.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// SwapChain::DxDevice
//
class SwapChain::DxDevice {
  private: common::ComPtr<ID2D1Device> d2d_device_;
  private: common::ComPtr<IDXGIDevice1> dxgi_device_;
  private: common::ComPtr<IDXGIFactory2> dxgi_factory_;

  public: DxDevice();
  public: ~DxDevice();

  public: ID2D1Device* d2d_device() const { return d2d_device_; }
  public: IDXGIDevice1* dxgi_device() const { return dxgi_device_; }
  public: IDXGIFactory* dxgi_factory() const { return dxgi_factory_; }

  public: common::ComPtr<IDXGISwapChain2> CreateSwapChain(
      const D2D1_SIZE_U& size);
  public: common::ComPtr<IDXGISwapChain2> CreateSwapChain(HWND hwnd);

  DISALLOW_COPY_AND_ASSIGN(DxDevice);
};

SwapChain::DxDevice::DxDevice() {
  // Create Direct 3D device.
  D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
  };

#if _DEBUG
  auto const d3d11_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                           D3D11_CREATE_DEVICE_SINGLETHREADED |
                           D3D11_CREATE_DEVICE_DEBUG;
#else
  auto const d3d11_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                           D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif
  common::ComPtr<ID3D11Device> d3d_device;
  COM_VERIFY(::D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
      d3d11_flags, nullptr, 0, D3D11_SDK_VERSION,
      &d3d_device, feature_levels, nullptr));
  COM_VERIFY(dxgi_device_.QueryFrom(d3d_device));

  common::ComPtr<IDXGIAdapter> dxgi_adapter;
  dxgi_device_->GetAdapter(&dxgi_adapter);

  common::ComPtr<IDXGIFactory2> dxgi_factory;
  dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory_));

  // Create d2d device
  COM_VERIFY(FactorySet::instance()->d2d1().CreateDevice(dxgi_device_,
                                                         &d2d_device_));
}

SwapChain::DxDevice::~DxDevice() {
}

common::ComPtr<IDXGISwapChain2>SwapChain::DxDevice::CreateSwapChain(
        const D2D1_SIZE_U& size) {
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
  swap_chain_desc.Width = size.width;
  swap_chain_desc.Height = size.height;
  swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1; // don't use multi-sampling
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;  // use double buffering to enable flip
  swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  common::ComPtr<IDXGISwapChain1> swap_chain1;
  COM_VERIFY(dxgi_factory_->CreateSwapChainForComposition(
      dxgi_device_, &swap_chain_desc, nullptr, &swap_chain1));
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
  // COM_VERIFY(swap_chain_->SetMaximumFrameLatency(1));
  return swap_chain2;
}

common::ComPtr<IDXGISwapChain2> SwapChain::DxDevice::CreateSwapChain(
    HWND hwnd) {
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
  swap_chain_desc.Scaling = DXGI_SCALING_NONE;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  common::ComPtr<IDXGISwapChain1> swap_chain1;
  COM_VERIFY(dxgi_factory_->CreateSwapChainForHwnd(
      dxgi_device_, hwnd, &swap_chain_desc, nullptr, nullptr, &swap_chain1));

  common::ComPtr<IDXGISwapChain2> swap_chain2;
  COM_VERIFY(swap_chain2.QueryFrom(swap_chain1));
  return swap_chain2;
}

//////////////////////////////////////////////////////////////////////
//
// SwapChain
//
SwapChain::SwapChain(DxDevice* device,
                     common::ComPtr<IDXGISwapChain2> swap_chain)
    : is_ready_(false), swap_chain_(swap_chain),
      swap_chain_waitable_(swap_chain_->GetFrameLatencyWaitableObject()) {
  COM_VERIFY(device->d2d_device()->CreateDeviceContext(
      D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context_));
  UpdateDeviceContext();
}

SwapChain::~SwapChain() {
  d2d_device_context_->SetTarget(nullptr);
  d2d_device_context_.reset();
}

void SwapChain::AddDirtyRect(const Rect& new_dirty_rect) {
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

SwapChain* SwapChain::Create(HWND hwnd) {
  DxDevice device;
  return new SwapChain(&device, device.CreateSwapChain(hwnd));
}

SwapChain* SwapChain::Create(const D2D1_SIZE_U& size) {
  DxDevice device;
  return new SwapChain(&device, device.CreateSwapChain(size));
}

void SwapChain::DidChangeBounds(const D2D1_SIZE_U& size) {
  d2d_device_context_->SetTarget(nullptr);
  COM_VERIFY(swap_chain_->ResizeBuffers(0u, size.width, size.height,
      DXGI_FORMAT_UNKNOWN,
      DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));
  UpdateDeviceContext();
}

bool SwapChain::IsReady() {
  if (is_ready_)
    return true;
  auto const wait = ::WaitForSingleObject(swap_chain_waitable_, 0);
  switch (wait){
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
  if (dirty_rects_.empty()) {
    DVLOG(0) << "SwapChain::Present: no dirty rects";
    return;
  }
  DXGI_PRESENT_PARAMETERS parameters = {0};
  parameters.DirtyRectsCount = dirty_rects_.size();
  parameters.pDirtyRects = dirty_rects_.data();
  parameters.pScrollRect = nullptr;
  parameters.pScrollOffset = nullptr;
  auto const flags = DXGI_PRESENT_DO_NOT_WAIT;
  COM_VERIFY(swap_chain_->Present1(0, flags, &parameters));
  dirty_rects_.clear();
  is_ready_ = false;
}

void SwapChain::UpdateDeviceContext() {
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

    float dpi_x, dpi_y;
    FactorySet::instance()->d2d1().GetDesktopDpi(&dpi_x, &dpi_y);
    auto const bitmap_properties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                          D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpi_x, dpi_y);

    common::ComPtr<ID2D1Bitmap1> d2d_back_buffer;
    COM_VERIFY(d2d_device_context_->CreateBitmapFromDxgiSurface(
        dxgi_back_buffer, bitmap_properties, &d2d_back_buffer));
    d2d_device_context_->SetTarget(d2d_back_buffer);
  }

  d2d_device_context_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
}

}  // namespace gfx
