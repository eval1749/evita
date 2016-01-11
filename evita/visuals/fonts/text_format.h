// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_TEXT_FORMAT_H_
#define EVITA_VISUALS_FONTS_TEXT_FORMAT_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace visuals {

class FloatSize;
class FontDescription;
class NativeTextFormat;

//////////////////////////////////////////////////////////////////////
//
// TextFormat
//
class TextFormat final {
 public:
  explicit TextFormat(const FontDescription& description);
  ~TextFormat();

  const FontDescription& font_description() const { return font_description_; }
  const NativeTextFormat& impl() const { return *impl_; }

  FloatSize ComputeMetrics(const base::string16& text) const;

  bool operator==(const TextFormat& other) const = delete;
  bool operator!=(const TextFormat& other) const = delete;

 private:
  const FontDescription& font_description_;
  std::unique_ptr<NativeTextFormat> impl_;

  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

std::ostream& operator<<(std::ostream& ostream, const TextFormat& text_format);

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_TEXT_FORMAT_H_
