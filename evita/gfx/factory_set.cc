// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/factory_set.h"

#include <dwrite.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace gfx {

namespace {

ID2D1Factory1& CreateD2D1Factory() {
  ID2D1Factory1* factory;
  D2D1_FACTORY_OPTIONS options;
#if _DEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
  options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
  COM_VERIFY(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options,
                                 &factory));
  return *factory;
}

IDWriteFactory& CreateDWriteFactory() {
  IDWriteFactory* factory;
  COM_VERIFY(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                   __uuidof(IDWriteFactory),
                                   reinterpret_cast<IUnknown**>(&factory)));
  return *factory;
}

IWICImagingFactory& CreateImageFactory() {
  IWICImagingFactory* factory;
  COM_VERIFY(::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
  return *factory;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FactorySet
//
FactorySet::FactorySet()
    : d2d1_factory_(CreateD2D1Factory()),
      dwrite_factory_(CreateDWriteFactory()),
      image_factory_(CreateImageFactory()) {
  SizeF dpi;
  d2d1_factory_->GetDesktopDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
}

FactorySet::~FactorySet() {}

// static
FactorySet* FactorySet::GetInstance() {
  return base::Singleton<FactorySet>::get();
}

}  // namespace gfx
