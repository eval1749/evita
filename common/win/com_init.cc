// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <objbase.h>

#include "common/win/com_init.h"

#include "common/win/com_verify.h"

namespace common {
namespace win {

ComInit::ComInit() {
  COM_VERIFY(::CoInitialize(nullptr));
}
ComInit::~ComInit() {
  ::CoUninitialize();
}

}  // namespace win
}  // namespace common
