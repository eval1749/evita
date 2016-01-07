// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_TOOLTIP_H_
#define EVITA_UI_TOOLTIP_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gfx/rect.h"

struct tagTOOLINFOW;
typedef struct tagTOOLINFOW TTTOOLINFOW;

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Tooltip
//
class Tooltip final {
 public:
  class ToolDelegate {
   public:
    virtual ~ToolDelegate();

    virtual base::string16 GetTooltipText() = 0;

   protected:
    ToolDelegate();

   private:
    DISALLOW_COPY_AND_ASSIGN(ToolDelegate);
  };

  Tooltip();
  ~Tooltip();

  void AddTool(ToolDelegate* tool_delegate);
  void DeleteTool(ToolDelegate* tool_delegate);
  void OnNotify(NMHDR* nmhdr);
  void Realize(HWND tool_hwnd);
  void SetToolBounds(ToolDelegate* tool_delegate, const gfx::Rect& bounds);

 private:
  void SendMessage(UINT message, TTTOOLINFOW* info);

  HWND tool_hwnd_;
  base::string16 tool_text_;
  HWND tooltip_hwnd_;

  DISALLOW_COPY_AND_ASSIGN(Tooltip);
};

}  // namespace ui

#endif  // EVITA_UI_TOOLTIP_H_
