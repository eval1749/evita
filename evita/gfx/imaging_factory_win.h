// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_IMAGING_FACTORY_WIN_H_
#define EVITA_GFX_IMAGING_FACTORY_WIN_H_

#include <wincodec.h>
#include <wrl/client.h>

#include "base/macros.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// ImagingFactory
//
class ImagingFactory {
 public:
  ImagingFactory();
  ~ImagingFactory();

  const Microsoft::WRL::ComPtr<IWICImagingFactory>& impl() { return impl_; }

  static ImagingFactory* GetInstance();

 private:
  Microsoft::WRL::ComPtr<IWICImagingFactory> impl_;

  DISALLOW_COPY_AND_ASSIGN(ImagingFactory);
};

}  // namespace gfx

#endif  // EVITA_GFX_IMAGING_FACTORY_WIN_H_
