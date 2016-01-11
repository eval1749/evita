// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_TEXT_FORMAT_FACTORY_H_
#define EVITA_VISUALS_FONTS_TEXT_FORMAT_FACTORY_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"

namespace visuals {

class FontDescription;
class TextFormat;

//////////////////////////////////////////////////////////////////////
//
// TextFormatFactory
//
class TextFormatFactory final {
 public:
  TextFormatFactory();
  ~TextFormatFactory();

  const TextFormat& Get(const FontDescription& font_description);

  static TextFormatFactory* GetInstance();

 private:
  std::unordered_map<const FontDescription*, std::unique_ptr<TextFormat>> map_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatFactory);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_TEXT_FORMAT_FACTORY_H_
