// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_GEOMETRY_INT_SIZE_H_
#define EVITA_GFX_BASE_GEOMETRY_INT_SIZE_H_

#include <iosfwd>

#include "evita/gfx/gfx_export.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// IntSize
//
class GFX_EXPORT IntSize final {
 public:
  IntSize(int width, int height);
  IntSize(const IntSize& other);
  IntSize();
  ~IntSize();

  IntSize& operator=(const IntSize& other);

  bool operator==(const IntSize& other) const;
  bool operator!=(const IntSize& other) const;

  int height() const { return height_; }
  int width() const { return width_; }

  bool IsEmpty() const { return width_ == 0 || height_ == 0; }

 private:
  int height_ = 0;
  int width_ = 0;
};

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream, const IntSize& size);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_GEOMETRY_INT_SIZE_H_
