// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BASE_H_
#define EVITA_GFX_BASE_H_

#include <dwrite.h>
#include <wincodec.h>

#include <memory>

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/logging.h"
#include "common/win/rect.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/dpi_handler.h"
#include "evita/gfx/rect_f.h"
#include "evita/gfx/simple_object.h"

namespace gfx {

typedef common::win::Point Point;
typedef common::win::Rect Rect;
typedef common::win::Size Size;

//////////////////////////////////////////////////////////////////////
//
// Graphics objects
//
class ColorF;
class Bitmap;
class Brush;
class FactorySet;
class FontFace;
class Canvas;
class TextFormat;
class TextLayout;

}  // namespace gfx

#endif  // EVITA_GFX_BASE_H_
