// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_vi_Pane_h)
#define INCLUDE_vi_Pane_h

#include "base/strings/string16.h"
#include "evita/li_util.h"
#include "evita/views/command_window.h"
#include "evita/views/window.h"

class Frame;

class Pane : public CommandWindow, public ChildNode_<Frame, Pane> {
  protected: typedef ui::Widget Widget;
  protected: typedef ui::Widget Widget;

  DECLARE_CASTABLE_CLASS(Pane, CommandWindow);

  private: const base::string16 name_;

  // ctor
  protected: Pane(std::unique_ptr<common::win::NativeWindow>&&);
  protected: Pane();
  public: ~Pane();

  // [A]
  public: virtual void Activate();

  // [G]
  public: virtual HCURSOR GetCursorAt(const common::win::Point&) const;
  public: Frame* GetFrame() const { return m_pParent; }
  public: const base::string16 GetName() const { return name_; }
  public: virtual base::string16 GetTitle() = 0;

  DISALLOW_COPY_AND_ASSIGN(Pane);
};

#endif //!defined(INCLUDE_vi_Pane_h)
