// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_UI_ROOT_WIDGET_H_
#define EVITA_UI_ROOT_WIDGET_H_

#include "common/memory/singleton.h"
#include "evita/ui/widget.h"

namespace ui {

class RootWidget final : public Widget, public common::Singleton<RootWidget> {
  DECLARE_CASTABLE_CLASS(RootWidget, Widget);

 public:
  RootWidget();
  ~RootWidget() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(RootWidget);
};

}  // namespace ui

#endif  // EVITA_UI_ROOT_WIDGET_H_
