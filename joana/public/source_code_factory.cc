// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>

#include "joana/public/source_code_factory.h"

#include "joana/public/memory/zone.h"

namespace joana {

SourceCode::Factory::Factory(Zone* zone) : zone_(zone) {}
SourceCode::Factory::~Factory() = default;

SourceCode* SourceCode::Factory::New(const base::FilePath& file_path,
                                     base::StringPiece16 file_contents) {
  auto* const characters =
      zone_->AllocateObjects<base::char16>(file_contents.size());
  std::memcpy(characters, file_contents.data(),
              file_contents.size() * sizeof(base::char16));
  return new SourceCode(file_path,
                        base::StringPiece16(characters, file_contents.size()));
}

}  // namespace joana
