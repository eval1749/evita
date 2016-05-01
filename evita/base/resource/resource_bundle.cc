// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "evita/base/resource/resource_bundle.h"

#include "base/memory/singleton.h"
#include "evita/base/resource/data_pack.h"

namespace base {

ResourceBundle::ResourceBundle() = default;
ResourceBundle::~ResourceBundle() = default;

void ResourceBundle::AddDataPackFromPath(const base::FilePath& path) {
  auto data_pack = std::make_unique<DataPack>();
  if (!data_pack->LoadFromPath(path))
    return;
  data_packs_.emplace_back(std::move(data_pack));
}

ResourceBundle* ResourceBundle::GetInstance() {
  return base::Singleton<ResourceBundle>::get();
}

base::StringPiece ResourceBundle::GetRawDatResource(int resource_id) const {
  if (resource_id <= 0 || resource_id > 0xFFFF)
    return base::StringPiece();
  const auto data_id = static_cast<uint16_t>(resource_id);
  for (const auto& data_pack : data_packs_) {
    const auto& string = data_pack->GetStringPiece(data_id);
    if (string.data())
      return string;
  }
  return base::StringPiece();
}

}  // namespace base
