// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/dpi_handler.h"

namespace gfx {

namespace {

float MultipleOf(float x, float unit) {
  return ::ceilf(x / unit) * unit;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DpiHandler
//
SizeF DpiHandler::AlignToPixel(const SizeF& size) const {
  DCHECK(!pixels_per_dip_.empty());
  return SizeF(MultipleOf(size.width, pixels_per_dip_.width),
               MultipleOf(size.height, pixels_per_dip_.height));
}

SizeF DpiHandler::CeilToPixel(const SizeF& size) const {
  return SizeF(::ceilf(size.width * pixels_per_dip_.width),
               ::ceilf(size.height * pixels_per_dip_.height));
}

SizeF DpiHandler::FloorToPixel(const SizeF& size) const {
  return SizeF(::floorf(size.width * pixels_per_dip_.width),
               ::floorf(size.height * pixels_per_dip_.height));
}

void DpiHandler::UpdateDpi(const SizeF& dpi) {
  dpi_ = dpi;
  // Note: All render targets except for ID2D1HwndRenderTarget, DPI values
  // are 96 DPI.
  float const default_dpi = 96.0f;
  pixels_per_dip_ = dpi_ / default_dpi;
  pixels_per_dip_ = SizeF(96.0f / dpi_.width, 96.0f / dpi_.height);
}

}  // namespace gfx
