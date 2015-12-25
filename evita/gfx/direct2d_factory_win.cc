// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/direct2d_factory_win.h"

#include "base/memory/singleton.h"
#include "common/win/com_verify.h"

#pragma comment(lib, "d2d1.lib")

namespace gfx {

namespace {

base::win::ScopedComPtr<ID2D1Factory1> CreateD2D1Factory() {
  base::win::ScopedComPtr<ID2D1Factory1> factory;
  D2D1_FACTORY_OPTIONS options;
#if _DEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
  options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
  COM_VERIFY(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options,
                                 factory.Receive()));
  return factory;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Direct2DFactory
//
Direct2DFactory::Direct2DFactory() : impl_(CreateD2D1Factory()) {
  SizeF dpi;
  impl_->GetDesktopDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
}

Direct2DFactory::~Direct2DFactory() {}

// static
SizeF Direct2DFactory::AlignToPixel(const SizeF& size) {
  return GetInstance()->DpiHandler::AlignToPixel(size);
}
// static
SizeF Direct2DFactory::CeilToPixel(const SizeF& size) {
  return GetInstance()->DpiHandler::CeilToPixel(size);
}

// static
Direct2DFactory* Direct2DFactory::GetInstance() {
  return base::Singleton<Direct2DFactory>::get();
}

}  // namespace gfx
