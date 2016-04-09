// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_IMAGING_ICON_UTIL_H_
#define EVITA_VISUALS_IMAGING_ICON_UTIL_H_

#include <memory>

#include "base/strings/string_piece.h"

namespace visuals {

class ImageBitmap;

const ImageBitmap* GetSmallIconForExtension(base::StringPiece16 extension);

}  // namespace visuals

#endif  // EVITA_VISUALS_IMAGING_ICON_UTIL_H_
