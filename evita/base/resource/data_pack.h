// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_RESOURCE_DATA_PACK_H_
#define EVITA_BASE_RESOURCE_DATA_PACK_H_

#include <stdint.h>

#include <memory>

#include "base/files/file.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/base/evita_base_export.h"

namespace base {

class FilePath;
class MemoryMappedFile;

class EVITA_BASE_EXPORT DataPack {
 public:
  DataPack();
  ~DataPack();

  bool LoadFromFile(base::File file);
  bool LoadFromPath(const base::FilePath& path);
  base::StringPiece GetStringPiece(uint16_t resource_id) const;

 private:
  bool LoadInternal(std::unique_ptr<base::MemoryMappedFile> mmap);

  std::unique_ptr<base::MemoryMappedFile> mmap_;
  size_t resource_count_ = 0;

  DISALLOW_COPY_AND_ASSIGN(DataPack);
};

}  // namespace base

#endif  // EVITA_BASE_RESOURCE_DATA_PACK_H_
