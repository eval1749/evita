// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/imaging_factory_win.h"

#include "base/memory/singleton.h"
#include "common/win/com_verify.h"

#pragma comment(lib, "windowscodecs.lib")

namespace gfx {

namespace {
Microsoft::WRL::ComPtr<IWICImagingFactory> CreateImagingFactory() {
  Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
  COM_VERIFY(::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
  return factory;
}
}  // namespace

ImagingFactory::ImagingFactory() : impl_(CreateImagingFactory()) {}

ImagingFactory::~ImagingFactory() {}

// static
ImagingFactory* ImagingFactory::GetInstance() {
  return base::Singleton<ImagingFactory>::get();
}

}  // namespace gfx
