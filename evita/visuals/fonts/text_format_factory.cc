// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/text_format_factory.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "evita/visuals/fonts/font_description.h"
#include "evita/visuals/fonts/text_format.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TextFormatFactory
//
TextFormatFactory::TextFormatFactory() {}
TextFormatFactory::~TextFormatFactory() {}

const TextFormat& TextFormatFactory::Get(
    const FontDescription& font_description) {
  const auto& present = map_.find(&font_description);
  if (present != map_.end())
    return *present->second;
  auto new_text_format = std::make_unique<TextFormat>(font_description);
  const auto& result =
      map_.emplace(&font_description, std::move(new_text_format));
  DCHECK(result.second) << font_description << " should not be in map.";
  return *result.first->second;
}

// static
TextFormatFactory* TextFormatFactory::GetInstance() {
  return base::Singleton<TextFormatFactory>::get();
}

}  // namespace visuals
