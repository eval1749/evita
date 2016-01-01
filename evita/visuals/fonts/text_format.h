// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_TEXT_FORMAT_H_
#define EVITA_VISUALS_FONTS_TEXT_FORMAT_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace visuals {

class NativeTextFormat;

//////////////////////////////////////////////////////////////////////
//
// TextFormat
//
class TextFormat final {
 public:
  TextFormat(const base::string16& font_face_name, float font_size);
  ~TextFormat();

  const NativeTextFormat& impl() const { return *impl_; }

  float ComputeWidth(const base::string16& text) const;

 private:
  std::unique_ptr<NativeTextFormat> impl_;

  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_TEXT_FORMAT_H_
