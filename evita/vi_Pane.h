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

#include "./li_util.h"
#include "./vi_CommandWindow.h"
#include "widgets/container_widget.h"

namespace gfx {
class Graphics;
}

class Frame;

class Pane : public CommandWindow_<Pane>, public ChildNode_<Frame, Pane> {
  protected: typedef widgets::ContainerWidget ContainerWidget;
  protected: typedef widgets::Widget Widget;

  DECLARE_CASTABLE_CLASS(Pane, CommandWindow);

  protected: uint             m_nActiveTick;
  protected: const char16*    m_pwszName;

  // ctor
  protected: Pane(std::unique_ptr<widgets::NaitiveWindow>&&);
  protected: Pane();

  // [A]
  public: virtual void Activate();

  // [G]
  public: uint GetActiveTick() const { return m_nActiveTick; }
  public: static const char* GetClass_() { return "Pane"; }
  public: virtual HCURSOR GetCursorAt(const gfx::Point&) const { return nullptr; }

  public: Frame*   GetFrame() const { return m_pParent; }
  public: const char16*  GetName()  const { return m_pwszName; }
  public: virtual int    GetTitle(char16* pwsz, int) = 0;

  // [I]
  public: virtual bool IsPane() const override { return true; }

  public: static bool Is_(const CommandWindow* p) { return p->IsPane(); }

  // [U]
  public: virtual void UpdateStatusBar() {}

  DISALLOW_COPY_AND_ASSIGN(Pane);
};

#endif //!defined(INCLUDE_vi_Pane_h)
