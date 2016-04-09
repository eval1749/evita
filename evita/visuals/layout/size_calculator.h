// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_SIZE_CALCULATOR_H_
#define EVITA_VISUALS_LAYOUT_SIZE_CALCULATOR_H_

#include "base/macros.h"

namespace gfx {
class FloatSize;
}

namespace visuals {

class Box;

//////////////////////////////////////////////////////////////////////
//
// SizeCalculator
//
class SizeCalculator final {
 public:
  SizeCalculator();
  ~SizeCalculator();

  gfx::FloatSize ComputeExtrinsicSize(const Box& box) const;
  gfx::FloatSize ComputeIntrinsicSize(const Box& box) const;
  gfx::FloatSize ComputePreferredSize(const Box& box) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(SizeCalculator);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_SIZE_CALCULATOR_H_
