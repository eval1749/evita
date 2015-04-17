// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/io_error.h"

namespace domapi {

IoError::IoError(int error_code) : error_code(error_code) {
}

}  // namespace domapi
