// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_DX_DEVICE_H_
#define EVITA_GFX_DX_DEVICE_H_

#include "base/basictypes.h"
#include "common/memory/singleton.h"
#include "common/win/scoped_comptr.h"

interface ID2D1Device;
interface IDXGIDevice3;
interface IDXGIFactory2;

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// DxDevice
//
class DxDevice : public common::Singleton<DxDevice> {
  DECLARE_SINGLETON_CLASS(DxDevice);

 public:
  // TODO(eval1749) We should destruct DxDevice by Singleton destructor.
  virtual ~DxDevice();

  ID2D1Device* d2d_device() const { return d2d_device_; }
  IDXGIDevice3* dxgi_device() const { return dxgi_device_; }
  IDXGIFactory2* dxgi_factory() const { return dxgi_factory_; }

 private:
  DxDevice();

  common::ComPtr<ID2D1Device> d2d_device_;
  common::ComPtr<IDXGIDevice3> dxgi_device_;
  common::ComPtr<IDXGIFactory2> dxgi_factory_;

  DISALLOW_COPY_AND_ASSIGN(DxDevice);
};

}  // namespace gfx

#endif  // EVITA_GFX_DX_DEVICE_H_
