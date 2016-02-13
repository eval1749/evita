// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "evita/views/icon_cache.h"

#include "build/build_config.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/visuals/imaging/icon_util.h"
#include "evita/visuals/imaging/image_bitmap.h"

#if OS_WIN
#include "evita/gfx/imaging_factory_win.h"
#include "evita/visuals/imaging/native_image_bitmap_win.h"
#else
#error "Unsupported target"
#endif

namespace views {

namespace {

std::unique_ptr<gfx::Bitmap> CreateBitmapFromImage(
    gfx::Canvas* canvas,
    const visuals::ImageBitmap& image) {
  common::ComPtr<IWICFormatConverter> converter;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateFormatConverter(
      &converter));
  const auto palette = static_cast<IWICPalette*>(nullptr);
  const auto alpha_threshold = 0.0f;
  COM_VERIFY(converter->Initialize(
      image.impl().get().get(), GUID_WICPixelFormat32bppPBGRA,
      WICBitmapDitherTypeNone, palette, alpha_threshold,
      WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY((*canvas)->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::make_unique<gfx::Bitmap>(canvas, bitmap);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IconCache
//
IconCache::IconCache() {}
IconCache::~IconCache() {}

void IconCache::Add(base::StringPiece16 name, int icon_index) {
  DCHECK_GE(icon_index, 0);
  map_.emplace(name.as_string(), icon_index);
}

std::unique_ptr<gfx::Bitmap> IconCache::BitmapFor(gfx::Canvas* canvas,
                                                  int icon_index) const {
  if (static_cast<size_t>(icon_index) >= bitmaps_.size())
    return CreateBitmapFromImage(canvas, *bitmaps_.front());
  return CreateBitmapFromImage(canvas, *bitmaps_[icon_index]);
}

int IconCache::GetIconForFileName(base::StringPiece16 file_name) {
  base::FilePath file_path(file_name);
  const auto& ext = file_path.Extension();
  if (!ext.empty()) {
    if (const auto icon_index = Intern(ext))
      return icon_index - 1;
  }

  const auto default_icon_index = Intern(L".txt");
  DCHECK_GE(default_icon_index, 1) << "No icon for .txt";

  Add(ext, default_icon_index - 1);
  return default_icon_index - 1;
}

// static
IconCache* IconCache::GetInstance() {
  return base::Singleton<IconCache>::get();
}

int IconCache::Intern(base::StringPiece16 name) {
  const auto& it = map_.find(name.as_string());
  if (it != map_.end())
    return it->second + 1;

  const auto icon_image = visuals::GetSmallIconForExtension(name);
  if (!icon_image)
    return 1;

  const auto icon_index = static_cast<int>(bitmaps_.size());
  bitmaps_.emplace_back(icon_image);
  Add(name, icon_index);
  return icon_index + 1;
}

}  // namespace views
