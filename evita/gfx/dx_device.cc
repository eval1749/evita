// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/dx_device.h"

#pragma warning(push)
#pragma warning(disable : 4061 4365 4917)
#include <d3d11_1.h>
#pragma warning(pop)

#include <dxgi1_3.h>

#include "evita/gfx/forward.h"

namespace gfx {

namespace {

common::ComPtr<ID2D1Factory1> CreateD2D1Factory() {
  common::ComPtr<ID2D1Factory1> factory;
  D2D1_FACTORY_OPTIONS options;
#if _DEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
  options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
  COM_VERIFY(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options,
                                 &factory));
  return std::move(factory);
}

}  // namespace

DxDevice::DxDevice() {
  // Create Direct 3D device.
  D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
  };

#if _DEBUG
  auto const d3d11_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                           D3D11_CREATE_DEVICE_SINGLETHREADED |
                           D3D11_CREATE_DEVICE_DEBUG;
#else
  auto const d3d11_flags =
      D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif
  common::ComPtr<ID3D11Device> d3d_device;
  COM_VERIFY(::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                 d3d11_flags, nullptr, 0, D3D11_SDK_VERSION,
                                 &d3d_device, feature_levels, nullptr));
  COM_VERIFY(dxgi_device_.QueryFrom(d3d_device));

  common::ComPtr<IDXGIAdapter> dxgi_adapter;
  dxgi_device_->GetAdapter(&dxgi_adapter);
  dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory_));

  // Create d2d device
  auto d2d1_factory = CreateD2D1Factory();
  COM_VERIFY(d2d1_factory->CreateDevice(dxgi_device_, &d2d_device_));
}

DxDevice::~DxDevice() {}

// static
DxDevice* DxDevice::instance() {
  static DxDevice* static_device;
  if (!static_device)
    static_device = new DxDevice();
  return static_device;
}

}  // namespace gfx
