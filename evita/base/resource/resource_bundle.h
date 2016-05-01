// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_RESOURCE_RESOURCE_BUNDLE_H_
#define EVITA_BASE_RESOURCE_RESOURCE_BUNDLE_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/base/evita_base_export.h"

namespace base {

class DataPack;
template <typename T>
struct DefaultSingletonTraits;
class FilePath;

class EVITA_BASE_EXPORT ResourceBundle {
 public:
  ~ResourceBundle();

  void AddDataPackFromPath(const base::FilePath& path);
  base::StringPiece GetRawDatResource(int resource_id) const;

  static ResourceBundle* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ResourceBundle>;

  ResourceBundle();

  std::vector<std::unique_ptr<DataPack>> data_packs_;

  DISALLOW_COPY_AND_ASSIGN(ResourceBundle);
};

}  // namespace base

#endif  // EVITA_BASE_RESOURCE_RESOURCE_BUNDLE_H_
