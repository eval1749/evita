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
#include "base/observer_list.h"
#include "base/logging.h"
#include "common/win/rect.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/color_f.h"
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

class DpiHandler {
 public:
  const SizeF& pixels_per_dip() const { return pixels_per_dip_; }
  SizeF AlignToPixel(const SizeF& size) const;
  SizeF CeilToPixel(const SizeF& size) const;
  SizeF FloorToPixel(const SizeF& size) const;

 protected:
  void UpdateDpi(const SizeF&);

 private:
  SizeF dpi_;
  SizeF pixels_per_dip_;
};

class FactorySet final : public common::ComInit,
                         public DpiHandler,
                         public Object {
 public:
  static ID2D1Factory1& d2d1() { return *instance()->d2d1_factory_; }
  static IDWriteFactory& dwrite() { return *instance()->dwrite_factory_; }
  static IWICImagingFactory& image() { return *instance()->image_factory_; }
  static FactorySet* instance() { return GetInstance(); }

  static SizeF AlignToPixel(const SizeF& size) {
    return instance()->DpiHandler::AlignToPixel(size);
  }
  static SizeF CeilToPixel(const SizeF& size) {
    return instance()->DpiHandler::CeilToPixel(size);
  }

  static FactorySet* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<FactorySet>;

  FactorySet();
  ~FactorySet();

  common::ComPtr<ID2D1Factory1> d2d1_factory_;
  common::ComPtr<IDWriteFactory> dwrite_factory_;
  common::ComPtr<IWICImagingFactory> image_factory_;

  DISALLOW_COPY_AND_ASSIGN(FactorySet);
};

}  // namespace gfx

#endif  // EVITA_GFX_BASE_H_
