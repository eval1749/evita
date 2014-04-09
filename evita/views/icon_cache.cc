// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/icon_cache.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "common/win/registry.h"

namespace views {

namespace {

base::string16 GetExtension(const base::string16& name,
                             const base::string16& default_extension) {
  auto const index = name.find_last_of('.');
  return index == base::string16::npos ? default_extension :
                                         name.substr(index);
}

HICON LoadIconFromRegistry(const base::string16& extension) {
  common::win::Registry extension_key(HKEY_CLASSES_ROOT, extension);
  if (!extension_key)
    return nullptr;

  auto const id_name = extension_key.GetValue();
  if (id_name.empty())
    return nullptr;

  common::win::Registry id_key(HKEY_CLASSES_ROOT, id_name + L"\\DefaultIcon");
  if (!id_key)
    return nullptr;;

  auto const icon_location = id_key.GetValue();
  if (icon_location.empty())
    return nullptr;

  auto const comma_pos = icon_location.find_last_of(',');
  if (comma_pos == base::string16::npos) {
    HICON hIcon;
    if (::ExtractIconExW(icon_location.c_str(), 0, nullptr, &hIcon, 1) != 1)
      return nullptr;
    return hIcon;
  }

  auto const icon_path = icon_location.substr(0, comma_pos);
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

} // namespace

IconCache::IconCache()
    : image_list_(::ImageList_Create(16, 16, ILC_COLOR32, 10, 10)) {
}

IconCache::~IconCache() {
  if (image_list_)
    ::ImageList_Destroy(image_list_);
}

void IconCache::Add(const base::string16& name, int icon_index) {
  DCHECK_GE(icon_index, 0);
  map_[name] = icon_index;
}

int IconCache::AddIcon(const base::string16& name, HICON icon) {
  auto const icon_index = ::ImageList_ReplaceIcon(image_list_, -1, icon);
  if (icon_index >= 0)
    Add(name, icon_index);
  return icon_index;
}

int IconCache::GetIconForFileName(const base::string16& file_name) const {
  CR_DEFINE_STATIC_LOCAL(const base::string16, default_ext, (L".txt"));

  const base::string16 ext = GetExtension(file_name, default_ext);
  if (auto const icon_index = IconCache::instance()->Intern(ext))
    return icon_index - 1;

  auto const default_icon_index = IconCache::instance()->Intern(default_ext);
  DCHECK(default_icon_index);

  IconCache::instance()->Add(ext, default_icon_index - 1);
  return default_icon_index - 1;
}

int IconCache::Intern(const base::string16& name) {
  auto const it = map_.find(name);
  if (it != map_.end())
    return it->second + 1;

  if (auto const icon = LoadIconFromRegistry(name)) {
    auto const index = AddIcon(name, icon) + 1;
    ::DestroyIcon(icon);
    return index;
  }

  return 0;
}

}  // namespace views
