// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_tooltip_h)
#define INCLUDE_evita_ui_tooltip_h

#include "base/strings/string16.h"

struct tagTOOLINFOW;
typedef struct tagTOOLINFOW TTTOOLINFOW;

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Tooltip
//
class Tooltip final {
  public: class ToolDelegate {
    public: ToolDelegate();
    public: virtual ~ToolDelegate();

    public: virtual base::string16 GetTooltipText() = 0;

    DISALLOW_COPY_AND_ASSIGN(ToolDelegate);
  };

  private: HWND tool_hwnd_;
  private: base::string16 tool_text_;
  private: HWND tooltip_hwnd_;

  public: Tooltip();
  public: ~Tooltip();

  public: void AddTool(ToolDelegate* tool_delegate);
  public: void DeleteTool(ToolDelegate* tool_delegate);
  public: void OnNotify(NMHDR* nmhdr);
  public: void Realize(HWND tool_hwnd);
  private: void SendMessage(UINT message, TTTOOLINFOW* info);
  public: void SetToolBounds(ToolDelegate* tool_delegate, const RECT& bounds);

  DISALLOW_COPY_AND_ASSIGN(Tooltip);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_tooltip_h)
