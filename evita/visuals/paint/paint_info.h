// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_PAINT_PAINT_INFO_H_
#define EVITA_VISUALS_PAINT_PAINT_INFO_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gfx/base/geometry/float_rect.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// PaintInfo
//
class PaintInfo final {
 public:
  PaintInfo(const gfx::FloatRect& cull_rect, const base::string16& debug_text);
  explicit PaintInfo(const gfx::FloatRect& cull_rect);
  ~PaintInfo();

  const gfx::FloatRect& cull_rect() const { return cull_rect_; }
  const base::string16& debug_text() const { return debug_text_; }

 private:
  const gfx::FloatRect cull_rect_;
  const base::string16 debug_text_;

  DISALLOW_COPY_AND_ASSIGN(PaintInfo);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_PAINT_PAINT_INFO_H_
