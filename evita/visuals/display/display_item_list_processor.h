// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_PROCESSOR_H_
#define EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_PROCESSOR_H_

#include <memory>
#include <vector>

#include "base/macros.h"

namespace gfx {
class Canvas;
}

namespace visuals {

class DisplayItemList;
class FloatRect;

//////////////////////////////////////////////////////////////////////
//
// DisplayItemListProcessor
//
class DisplayItemListProcessor final {
 public:
  DisplayItemListProcessor();
  ~DisplayItemListProcessor();

  void Paint(gfx::Canvas* canvas, std::unique_ptr<DisplayItemList> list);

 private:
  std::unique_ptr<DisplayItemList> last_items_;

  DISALLOW_COPY_AND_ASSIGN(DisplayItemListProcessor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_DISPLAY_ITEM_LIST_PROCESSOR_H_
