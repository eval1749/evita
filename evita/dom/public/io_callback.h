// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_callback_h)
#define INCLUDE_evita_dom_public_io_callback_h

#include "base/time/time.h"

namespace domapi {

struct QueryFileStatusCallbackData {
  int error_code;
  int file_size;
  bool is_directory;
  bool is_symlink;
  base::Time last_write_time;
  bool readonly;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_callback_h)
