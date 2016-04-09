// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/visual_window.h"

#include "base/trace_event/trace_event.h"
#include "evita/gfx/canvas.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

using DisplayItemList = visuals::DisplayItemList;
using DisplayItemListProcessor = visuals::DisplayItemListProcessor;

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
VisualWindow::VisualWindow(WindowId window_id)
    : CanvasContentWindow(window_id) {}

VisualWindow::~VisualWindow() {}

void VisualWindow::Paint(std::unique_ptr<DisplayItemList> display_item_list) {
  if (!visible() || !canvas()->UpdateReadyState())
    return;
  TRACE_EVENT_WITH_FLOW0("visuals", "VisualWindow::Paint", window_id(),
                         TRACE_EVENT_FLAG_FLOW_IN);
  DisplayItemListProcessor processor;
  processor.Paint(canvas(), std::move(display_item_list));
  NotifyUpdateContent();
}

}  // namespace views
