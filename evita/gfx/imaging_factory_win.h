// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_IMAGING_FACTORY_WIN_H_
#define EVITA_GFX_IMAGING_FACTORY_WIN_H_

#include <wincodec.h>

#include "base/macros.h"
#include "base/win/scoped_comptr.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// ImagingFactory
//
class ImagingFactory {
 public:
  ImagingFactory();
  ~ImagingFactory();

  const base::win::ScopedComPtr<IWICImagingFactory>& impl() { return impl_; }

  static ImagingFactory* GetInstance();

 private:
  base::win::ScopedComPtr<IWICImagingFactory> impl_;

  DISALLOW_COPY_AND_ASSIGN(ImagingFactory);
};

}  // namespace gfx

#endif  // EVITA_GFX_IMAGING_FACTORY_WIN_H_
