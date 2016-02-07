// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/text_area_display_item.h"

#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// TextAreaDisplayItem
//
TextAreaDisplayItem::TextAreaDisplayItem(
    scoped_refptr<paint::View> paint_view,
    std::unique_ptr<DisplayItemList> display_item_list)
    : display_item_list_(std::move(display_item_list)),
      paint_view_(paint_view) {}

TextAreaDisplayItem::~TextAreaDisplayItem() {}

}  // namespace domapi
