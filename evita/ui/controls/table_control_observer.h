// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_UI_CONTROLS_TABLE_CONTROL_OBSERVER_H_
#define EVITA_UI_CONTROLS_TABLE_CONTROL_OBSERVER_H_

#include <vector>

namespace ui {

class KeyEvent;
class MouseEvent;
class TableControlObserver;

class TableControlObserver {
 public:
  virtual ~TableControlObserver();

  virtual void OnKeyPressed(const KeyEvent&);
  virtual void OnMousePressed(const MouseEvent&);
  virtual void OnSelectionChanged();

 protected:
  TableControlObserver();
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_TABLE_CONTROL_OBSERVER_H_
