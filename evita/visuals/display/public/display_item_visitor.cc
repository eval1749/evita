// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/public/display_item_visitor.h"

#include "evita/visuals/display/public/display_items.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DisplayItemVisitor
//
DisplayItemVisitor::DisplayItemVisitor() {}
DisplayItemVisitor::~DisplayItemVisitor() {}

void DisplayItemVisitor::Visit(const DisplayItem& item) {
  Visit(&const_cast<DisplayItem&>(item));
}

void DisplayItemVisitor::Visit(DisplayItem* item) {
  item->Accept(this);
}

#define V(name)                                                 \
  void name##DisplayItem::Accept(DisplayItemVisitor* visitor) { \
    visitor->Visit##name(this);                                 \
  }
FOR_EACH_DISPLAY_ITEM(V)
#undef V

}  // namespace visuals
