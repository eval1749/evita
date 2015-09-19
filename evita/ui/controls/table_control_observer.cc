// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/controls/table_control_observer.h"

namespace ui {

TableControlObserver::TableControlObserver() {}

TableControlObserver::~TableControlObserver() {}

void TableControlObserver::OnKeyPressed(const KeyEvent&) {}

void TableControlObserver::OnMousePressed(const MouseEvent&) {}

void TableControlObserver::OnSelectionChanged() {}

}  // namespace ui
