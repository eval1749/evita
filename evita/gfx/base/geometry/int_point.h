// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_GEOMETRY_INT_POINT_H_
#define EVITA_GFX_BASE_GEOMETRY_INT_POINT_H_

#include <iosfwd>

#include "evita/gfx/gfx_export.h"

namespace gfx {

class IntSize;

//////////////////////////////////////////////////////////////////////
//
// IntPoint
//
class GFX_EXPORT IntPoint final {
 public:
  IntPoint(int x, int y);
  IntPoint(const IntPoint& other);
  IntPoint();
  ~IntPoint();

  IntPoint& operator=(const IntPoint& other);

  bool operator==(const IntPoint& other) const;
  bool operator!=(const IntPoint& other) const;

  IntPoint operator+(const IntSize& size) const;
  IntPoint operator-(const IntSize& size) const;

  int x() const { return x_; }
  int y() const { return y_; }

 private:
  int x_ = 0;
  int y_ = 0;
};

GFX_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                    const IntPoint& point);

}  // namespace gfx

#endif  // EVITA_GFX_BASE_GEOMETRY_INT_POINT_H_
