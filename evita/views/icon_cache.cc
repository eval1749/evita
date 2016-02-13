// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <shellapi.h>

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

std::unique_ptr<gfx::Bitmap> CreateBitmapFromIcon(gfx::Canvas* canvas,
                                                  HICON hIcon) {
  common::ComPtr<IWICBitmap> icon;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateBitmapFromHICON(
      hIcon, &icon));
  common::ComPtr<IWICFormatConverter> converter;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateFormatConverter(
      &converter));
  COM_VERIFY(converter->Initialize(icon, GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapDitherTypeNone, nullptr, 0,
                                   WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY((*canvas)->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::make_unique<gfx::Bitmap>(canvas, bitmap);
}

base::string16 GetExtension(const base::string16& name,
                            const base::string16& default_extension) {
  const auto index = name.find_last_of('.');
  return index == base::string16::npos ? default_extension : name.substr(index);
}

HICON LoadIconFromRegistry(const base::string16& extension) {
  common::win::Registry extension_key(HKEY_CLASSES_ROOT, extension);
  if (!extension_key)
    return nullptr;

  const auto id_name = extension_key.GetValue();
  if (id_name.empty())
    return nullptr;

  common::win::Registry id_key(HKEY_CLASSES_ROOT, id_name + L"\\DefaultIcon");
  if (!id_key)
    return nullptr;

  const auto icon_location = id_key.GetValue();
  if (icon_location.empty())
    return nullptr;

  const auto comma_pos = icon_location.find_last_of(',');
  if (comma_pos == base::string16::npos) {
    HICON hIcon;
    if (::ExtractIconExW(icon_location.c_str(), 0, nullptr, &hIcon, 1) != 1)
      return nullptr;
    return hIcon;
  }

  const auto icon_path = icon_location.substr(0, comma_pos);
  int icon_index;
  if (!base::StringToInt(icon_location.substr(comma_pos + 1), &icon_index)) {
    DVLOG(0) << "Bad icon index: " << icon_location;
    return nullptr;
  }

  // Note: ExtractIconEx expands environment variables in
  // pathname, e.g. "%lsystemRoot%\system32\imageres.dll,-102".
  HICON hIcon;
  if (::ExtractIconExW(icon_path.c_str(), icon_index, nullptr, &hIcon, 1) != 1)
    return nullptr;
  return hIcon;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IconCache
//
IconCache::IconCache()
    : image_list_(::ImageList_Create(16, 16, ILC_COLOR32, 10, 10)) {}

IconCache::~IconCache() {
  if (!image_list_)
    return;
  ::ImageList_Destroy(image_list_);
}

void IconCache::Add(const base::string16& name, int icon_index) {
  DCHECK_GE(icon_index, 0);
  map_.emplace(name, icon_index);
}

int IconCache::AddIcon(const base::string16& name, HICON icon) {
  const auto icon_index = ::ImageList_ReplaceIcon(image_list_, -1, icon);
  if (icon_index >= 0)
    Add(name, icon_index);
  return icon_index;
}

std::unique_ptr<gfx::Bitmap> IconCache::BitmapFor(gfx::Canvas* canvas,
                                                  int icon_index) const {
  // Note: ILD_TRANSPARENT doesn't effect.
  // Note: ILD_DPISCALE makes background black.
  base::win::ScopedHICON icon(::ImageList_GetIcon(image_list_, icon_index, 0));
  if (!icon.is_valid())
    return std::unique_ptr<gfx::Bitmap>();
  return CreateBitmapFromIcon(canvas, icon.get());
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

  if (const auto icon = LoadIconFromRegistry(name)) {
    const auto index = AddIcon(name, icon) + 1;
    ::DestroyIcon(icon);
    return index;
  }

  return 0;
}

}  // namespace views
