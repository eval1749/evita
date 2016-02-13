// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <shellapi.h>
#include <stdint.h>

#include "evita/views/icon_cache.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/string_number_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "common/win/registry.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/imaging_factory_win.h"

namespace views {

namespace {

base::string16 GetExtension(const base::string16& name,
                            const base::string16& default_extension) {
  const auto index = name.find_last_of('.');
  return index == base::string16::npos ? default_extension : name.substr(index);
}

base::win::ScopedHICON LoadIconFromRegistry(const base::string16& extension) {
  common::win::Registry extension_key(HKEY_CLASSES_ROOT, extension);
  if (!extension_key)
    return base::win::ScopedHICON();

  const auto id_name = extension_key.GetValue();
  if (id_name.empty())
    return base::win::ScopedHICON();

  common::win::Registry id_key(HKEY_CLASSES_ROOT, id_name + L"\\DefaultIcon");
  if (!id_key)
    return base::win::ScopedHICON();

  const auto icon_location = id_key.GetValue();
  if (icon_location.empty())
    return base::win::ScopedHICON();

  const auto comma_pos = icon_location.find_last_of(',');
  if (comma_pos == base::string16::npos) {
    HICON hIcon;
    if (::ExtractIconExW(icon_location.c_str(), 0, nullptr, &hIcon, 1) != 1)
      return base::win::ScopedHICON();
    return base::win::ScopedHICON(hIcon);
  }

  const auto icon_path = icon_location.substr(0, comma_pos);
  int icon_index;
  if (!base::StringToInt(icon_location.substr(comma_pos + 1), &icon_index)) {
    DVLOG(0) << "Bad icon index: " << icon_location;
    return base::win::ScopedHICON();
  }

  // Note: ExtractIconEx expands environment variables in
  // pathname, e.g. "%lsystemRoot%\system32\imageres.dll,-102".
  HICON hIcon;
  if (::ExtractIconExW(icon_path.c_str(), icon_index, nullptr, &hIcon, 1) != 1)
    return base::win::ScopedHICON();
  return base::win::ScopedHICON(hIcon);
}

}  // namespace

using IconData = IconCache::IconData;

//////////////////////////////////////////////////////////////////////
//
// IconCache::IconData
//
class IconCache::IconData final {
 public:
  explicit IconData(const common::ComPtr<IWICBitmap>& bitmap);
  ~IconData() = default;

  std::unique_ptr<gfx::Bitmap> ToBitmap(gfx::Canvas* canvas) const;

 private:
  common::ComPtr<IWICBitmap> bitmap_;

  DISALLOW_COPY_AND_ASSIGN(IconData);
};

IconData::IconData(const common::ComPtr<IWICBitmap>& bitmap)
    : bitmap_(bitmap) {}

std::unique_ptr<gfx::Bitmap> IconData::ToBitmap(gfx::Canvas* canvas) const {
  common::ComPtr<IWICFormatConverter> converter;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateFormatConverter(
      &converter));
  COM_VERIFY(converter->Initialize(bitmap_, GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0,
                                   WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY((*canvas)->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::make_unique<gfx::Bitmap>(canvas, bitmap);
}

//////////////////////////////////////////////////////////////////////
//
// IconCache
//
IconCache::IconCache() {}
IconCache::~IconCache() {}

void IconCache::Add(const base::string16& name, int icon_index) {
  DCHECK_GE(icon_index, 0);
  map_.emplace(name, icon_index);
}

std::unique_ptr<gfx::Bitmap> IconCache::BitmapFor(gfx::Canvas* canvas,
                                                  int icon_index) const {
  if (static_cast<size_t>(icon_index) >= bitmaps_.size())
    return bitmaps_.front()->ToBitmap(canvas);
  return bitmaps_[icon_index]->ToBitmap(canvas);
}

int IconCache::GetIconForFileName(const base::string16& file_name) {
  const base::string16 ext = GetExtension(file_name, L".txt");
  if (const auto icon_index = Intern(ext))
    return icon_index - 1;

  const auto default_icon_index = Intern(L".txt");
  DCHECK(default_icon_index);

  Add(ext, default_icon_index - 1);
  return default_icon_index - 1;
}

// static
IconCache* IconCache::GetInstance() {
  return base::Singleton<IconCache>::get();
}

int IconCache::Intern(const base::string16& name) {
  const auto it = map_.find(name);
  if (it != map_.end())
    return it->second + 1;

  const auto icon = LoadIconFromRegistry(name);
  if (!icon.is_valid())
    return 1;

  common::ComPtr<IWICBitmap> bitmap;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateBitmapFromHICON(
      icon.get(), &bitmap));
  const auto icon_index = static_cast<int>(bitmaps_.size());
  bitmaps_.emplace_back(new IconData(bitmap));
  Add(name, icon_index);
  return icon_index + 1;
}

}  // namespace views
