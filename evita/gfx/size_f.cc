// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/size_f.h"

std::ostream& operator<<(std::ostream& ostream, const gfx::SizeF& size) {
  return ostream << size.width << "x" << size.height;
}

std::ostream& operator<<(std::ostream& ostream, const gfx::SizeF* size) {
  if (!size)
    return ostream << "(null)";
  return ostream <<*size;
}
