// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_DPI_HANDLER_H_
#define EVITA_GFX_DPI_HANDLER_H_

#include "evita/gfx/size_f.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// DpiHandler
//
class DpiHandler {
 public:
  const SizeF& pixels_per_dip() const { return pixels_per_dip_; }
  SizeF AlignToPixel(const SizeF& size) const;
  SizeF CeilToPixel(const SizeF& size) const;
  SizeF FloorToPixel(const SizeF& size) const;

 protected:
  void UpdateDpi(const SizeF&);

 private:
  SizeF dpi_;
  SizeF pixels_per_dip_;
};

}  // namespace gfx

#endif  // EVITA_GFX_DPI_HANDLER_H_
