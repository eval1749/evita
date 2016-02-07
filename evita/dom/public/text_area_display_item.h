// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_TEXT_AREA_DISPLAY_ITEM_H_
#define EVITA_DOM_PUBLIC_TEXT_AREA_DISPLAY_ITEM_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"

namespace paint {
class View;
}

namespace visuals {
class DisplayItemList;
}

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// TextAreaDisplayItem
//
class TextAreaDisplayItem final {
  using DisplayItemList = visuals::DisplayItemList;

 public:
  TextAreaDisplayItem(scoped_refptr<paint::View> paint_view,
                      std::unique_ptr<DisplayItemList> display_item_list);
  ~TextAreaDisplayItem();

  std::unique_ptr<DisplayItemList> display_item_list() {
    return std::move(display_item_list_);
  }
  const scoped_refptr<paint::View>& paint_view() { return paint_view_; }

 private:
  std::unique_ptr<DisplayItemList> display_item_list_;
  scoped_refptr<paint::View> paint_view_;

  DISALLOW_COPY_AND_ASSIGN(TextAreaDisplayItem);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_TEXT_AREA_DISPLAY_ITEM_H_
