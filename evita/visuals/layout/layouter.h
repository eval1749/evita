// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_LAYOUTER_H_
#define EVITA_VISUALS_LAYOUT_LAYOUTER_H_

#include "base/macros.h"

namespace visuals {

class Box;
class FloatRect;

//////////////////////////////////////////////////////////////////////
//
// Layouter
//
class Layouter final {
 public:
  Layouter();
  ~Layouter();

  void Layout(Box* box, const FloatRect& bounds);

 private:
  DISALLOW_COPY_AND_ASSIGN(Layouter);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_LAYOUTER_H_
