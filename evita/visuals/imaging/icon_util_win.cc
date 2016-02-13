// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <shellapi.h>

#include <memory>
#include <unordered_map>

#include "evita/visuals/imaging/icon_util.h"

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/strings/string_number_conversions.h"
#include "base/win/scoped_gdi_object.h"
#include "common/win/registry.h"
#include "evita/visuals/imaging/image_bitmap.h"
#include "evita/visuals/imaging/native_image_bitmap_win.h"

namespace visuals {

namespace {

base::win::ScopedHICON LoadSmallIconFromRegistry(
    base::StringPiece16 file_extension) {
  const auto& extension = file_extension.as_string();
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

//////////////////////////////////////////////////////////////////////
//
// SmallIconCache
//
class SmallIconCache final {
 public:
  SmallIconCache() = default;
  ~SmallIconCache() = default;

  const ImageBitmap* Add(base::StringPiece16 extension,
                         std::unique_ptr<ImageBitmap> bitmap);
  const ImageBitmap* Get(base::StringPiece16 extension);

  static SmallIconCache* GetInstance();

 private:
  std::unordered_map<base::string16, std::unique_ptr<ImageBitmap>> map_;

  DISALLOW_COPY_AND_ASSIGN(SmallIconCache);
};

const ImageBitmap* SmallIconCache::Add(base::StringPiece16 extension,
                                       std::unique_ptr<ImageBitmap> bitmap) {
  const auto& result = map_.emplace(extension.as_string(), std::move(bitmap));
  DCHECK(result.second);
  return result.first->second.get();
}

const ImageBitmap* SmallIconCache::Get(base::StringPiece16 extension) {
  const auto& it = map_.find(extension.as_string());
  return it == map_.end() ? nullptr : it->second.get();
}

SmallIconCache* SmallIconCache::GetInstance() {
  return base::Singleton<SmallIconCache>::get();
}

}  // namespace

const ImageBitmap* GetSmallIconForExtension(base::StringPiece16 extension) {
  if (const auto present = SmallIconCache::GetInstance()->Get(extension))
    return present;
  const auto& icon = LoadSmallIconFromRegistry(extension);
  if (!icon.is_valid())
    return nullptr;
  auto data = std::make_unique<NativeImageBitmap>(icon);
  return SmallIconCache::GetInstance()->Add(
      extension, std::make_unique<ImageBitmap>(std::move(data)));
}

}  // namespace visuals
