// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_SOURCE_CODE_FACTORY_H_
#define JOANA_BASE_SOURCE_CODE_FACTORY_H_

#include "joana/base/source_code.h"

namespace joana {

class Zone;

class JOANA_BASE_EXPORT SourceCode::Factory {
 public:
  explicit Factory(Zone* zone);
  ~Factory();

  const SourceCode& New(const base::FilePath& file_path,
                        base::StringPiece16 file_contents);

 private:
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace joana

#endif  // JOANA_BASE_SOURCE_CODE_FACTORY_H_
