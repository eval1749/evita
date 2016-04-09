// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_
#define EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace gfx {
class FloatPoint;
class FloatRect;
class FloatSize;
}

namespace visuals {

class NativeTextLayout;
class TextFormat;

//////////////////////////////////////////////////////////////////////
//
// TextLayout
//
class TextLayout final {
 public:
  TextLayout(const TextFormat& text_format,
             const base::string16& text,
             const gfx::FloatSize& size);
  TextLayout(const TextLayout& other);
  TextLayout(TextLayout&& other);
  ~TextLayout();

  bool operator==(const TextLayout& other) const;
  bool operator!=(const TextLayout& other) const;

  const NativeTextLayout& impl() const { return *impl_; }

  gfx::FloatSize GetMetrics() const;
  size_t HitTestPoint(const gfx::FloatPoint& point) const;
  gfx::FloatRect HitTestTextPosition(size_t offset) const;

 private:
  std::unique_ptr<NativeTextLayout> impl_;

  DISALLOW_ASSIGN(TextLayout);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_
