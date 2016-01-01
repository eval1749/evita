// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_
#define EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace visuals {

class FloatSize;
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
             const FloatSize& size);
  ~TextLayout();

  FloatSize GetMetrics() const;

 private:
  std::unique_ptr<NativeTextLayout> impl_;

  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_TEXT_LAYOUT_H_
