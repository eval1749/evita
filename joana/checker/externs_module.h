// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_CHECKER_EXTERNS_MODULE_H_
#define JOANA_CHECKER_EXTERNS_MODULE_H_

#include <vector>

namespace joana {

struct ExternsFile {
  const char* name;
  size_t content_size;
  const char* content;
};

struct ExternsModule {
  const char* name;
  std::vector<ExternsFile> files;
};

}  // namespace joana

#endif  // JOANA_CHECKER_EXTERNS_MODULE_H_
