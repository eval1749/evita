// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_IO_ERROR_H_
#define EVITA_DOM_PUBLIC_IO_ERROR_H_

namespace domapi {

struct IoError final {
  int error_code;

  explicit IoError(int error_code);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_IO_ERROR_H_
