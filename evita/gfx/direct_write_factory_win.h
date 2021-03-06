// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_DIRECT_WRITE_FACTORY_WIN_H_
#define EVITA_GFX_DIRECT_WRITE_FACTORY_WIN_H_

#include <dwrite.h>
#include <wrl/client.h>

#include "base/macros.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// DirectWriteFactory
//
class DirectWriteFactory {
 public:
  DirectWriteFactory();
  ~DirectWriteFactory();

  const Microsoft::WRL::ComPtr<IDWriteFactory>& impl() { return impl_; }

  static DirectWriteFactory* GetInstance();

 private:
  Microsoft::WRL::ComPtr<IDWriteFactory> impl_;

  DISALLOW_COPY_AND_ASSIGN(DirectWriteFactory);
};

}  // namespace gfx

#endif  // EVITA_GFX_DIRECT_WRITE_FACTORY_WIN_H_
