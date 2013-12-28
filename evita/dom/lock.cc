// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/lock.h"

namespace dom {

Lock::Lock()
    : lock_(new base::Lock()),
      locker_filename_(nullptr),
      locker_line_number_(0) {
}

}  // namespace dom
