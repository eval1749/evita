// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_DIRECT2D_FACTORY_WIN_H_
#define EVITA_GFX_DIRECT2D_FACTORY_WIN_H_

#include "base/macros.h"
#include "base/win/scoped_comptr.h"
#include "evita/gfx/dpi_handler.h"
#include "evita/gfx/size_f.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// Direct2DFactory
//
class Direct2DFactory final : public DpiHandler {
 public:
  Direct2DFactory();
  ~Direct2DFactory();

  const base::win::ScopedComPtr<ID2D1Factory1>& impl() const { return impl_; }

  static SizeF AlignToPixel(const SizeF& size);
  static SizeF CeilToPixel(const SizeF& size);
  static Direct2DFactory* GetInstance();

 private:
  base::win::ScopedComPtr<ID2D1Factory1> impl_;

  DISALLOW_COPY_AND_ASSIGN(Direct2DFactory);
};

}  // namespace gfx

#endif  // EVITA_GFX_DIRECT2D_FACTORY_WIN_H_
