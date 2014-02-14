//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Pane.h#1 $
//
#if !defined(INCLUDE_vi_Pane_h)
#define INCLUDE_vi_Pane_h

#include "base/strings/string16.h"
#include "evita/li_util.h"
#include "evita/views/command_window.h"
#include "evita/views/window.h"

namespace gfx {
class Graphics;
}

class Frame;

class Pane : public CommandWindow, public ChildNode_<Frame, Pane> {
  protected: typedef ui::Widget Widget;
  protected: typedef ui::Widget Widget;

  DECLARE_CASTABLE_CLASS(Pane, CommandWindow);

  protected: const char16*    m_pwszName;

  // ctor
  protected: Pane(std::unique_ptr<common::win::NativeWindow>&&);
  protected: Pane();

  // [A]
  public: virtual void Activate();

  // [G]
  public: virtual HCURSOR GetCursorAt(const common::win::Point&) const {
    return nullptr;
  }

  public: Frame*   GetFrame() const { return m_pParent; }
  public: const char16*  GetName()  const { return m_pwszName; }
  public: virtual base::string16 GetTitle() = 0;

  // [U]
  public: virtual void UpdateStatusBar() {}

  DISALLOW_COPY_AND_ASSIGN(Pane);
};

#endif //!defined(INCLUDE_vi_Pane_h)
