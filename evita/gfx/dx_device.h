// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_dx_device_h)
#define INCLUDE_evita_gfx_dx_device_h

#include "base/basictypes.h"
#include "common/win/scoped_comptr.h"

interface ID2D1Device;
interface IDXGIDevice2;
interface IDXGIFactory2;

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// DxDevice
//
class DxDevice {
  private: common::ComPtr<ID2D1Device> d2d_device_;
  private: common::ComPtr<IDXGIDevice2> dxgi_device_;
  private: common::ComPtr<IDXGIFactory2> dxgi_factory_;

  public: DxDevice();
  public: ~DxDevice();

  public: ID2D1Device* d2d_device() const { return d2d_device_; }
  public: IDXGIDevice2* dxgi_device() const { return dxgi_device_; }
  public: IDXGIFactory2* dxgi_factory() const { return dxgi_factory_; }

  DISALLOW_COPY_AND_ASSIGN(DxDevice);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_dx_device_h)
