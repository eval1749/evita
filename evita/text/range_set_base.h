// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_set_base_h)
#define INCLUDE_evita_text_range_set_base_h

#include "evita/precomp.h"

namespace text {

class RangeBase;

class RangeSetBase {
  protected: RangeSetBase();
  protected: ~RangeSetBase();

  protected: void set_range(RangeBase* range, Posn start, Posn end);
  protected: void set_range_end(RangeBase* range, Posn end);
  protected: void set_range_start(RangeBase* range, Posn start);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_range_set_base_h)
