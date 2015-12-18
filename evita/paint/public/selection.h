// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_SELECTION_H_
#define EVITA_PAINT_PUBLIC_SELECTION_H_

#include <unordered_set>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx_base.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection final : public base::RefCounted<Selection> {
 public:
  Selection(const gfx::ColorF color,
            const std::unordered_set<gfx::RectF> bounds_set);
  ~Selection();

  const gfx::ColorF color() const { return color_; }
  const std::unordered_set<gfx::RectF>& bounds_set() const {
    return bounds_set_;
  }

  bool HasBounds(const gfx::RectF& bounds) const;

 private:
  friend class base::RefCounted<Selection>;

  std::unordered_set<gfx::RectF> bounds_set_;
  const gfx::ColorF color_;

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_SELECTION_H_
