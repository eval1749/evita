// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_views_command_window_h)
#define INCLUDE_evita_views_command_window_h

#include "evita/views/window.h"

//////////////////////////////////////////////////////////////////////
//
// CommandWindow
//
class CommandWindow : public views::Window {
  DECLARE_CASTABLE_CLASS(CommandWindow, views::Window);

  protected: explicit CommandWindow(views::WindowId window_id);
  protected: virtual ~CommandWindow();

  // ui::Widget
  protected: void virtual DidSetFocus() override;

  DISALLOW_COPY_AND_ASSIGN(CommandWindow);
};

#endif //!defined(INCLUDE_evita_views_command_window_h)
