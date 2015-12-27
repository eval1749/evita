// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_VISITOR_H_
#define EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_VISITOR_H_

#include "base/macros.h"
#include "evita/visuals/display/public/display_items_forward.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DisplayItemVisitor
//
class DisplayItemVisitor {
 public:
  void Visit(const DisplayItem& item);
  void Visit(DisplayItem* item);

#define V(name) virtual void Visit##name(name##DisplayItem* item) = 0;
  FOR_EACH_DISPLAY_ITEM(V)
#undef V

 protected:
  DisplayItemVisitor();
  virtual ~DisplayItemVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(DisplayItemVisitor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEM_VISITOR_H_
