// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_PAINT_PAINTER_H_
#define EVITA_VISUALS_PAINT_PAINTER_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class DisplayItemList;
class PaintInfo;
class RootBox;

//////////////////////////////////////////////////////////////////////
//
// Painter
//
class Painter final {
 public:
  Painter();
  ~Painter();

  std::unique_ptr<DisplayItemList> Paint(const PaintInfo& paint_info,
                                         const RootBox& root_box);

 private:
  DISALLOW_COPY_AND_ASSIGN(Painter);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_PAINT_PAINTER_H_
