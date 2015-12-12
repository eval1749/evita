// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_CONTENT_WINDOW_H_
#define EVITA_VIEWS_CONTENT_WINDOW_H_

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/ui/compositor/layer_owner_delegate.h"
#include "evita/views/window.h"

class Frame;

namespace gfx {
class Canvas;
}

namespace ui {
class MouseEvent;
}

namespace views {

class ContentObserver;

class ContentWindow : public Window, public ui::LayerOwnerDelegate {
  DECLARE_CASTABLE_CLASS(ContentWindow, Window);

 public:
  ~ContentWindow() override;

  void Activate();
  void AddObserver(ContentObserver* observer);
  virtual void MakeSelectionVisible() = 0;
  void RemoveObserver(ContentObserver* observer);

 protected:
  explicit ContentWindow(views::WindowId window_id);

  void NotifyUpdateContent();

  // ui::Widget
  void DidChangeHierarchy() override;
  void DidRealize() override;
  void DidSetFocus(ui::Widget*) override;

 private:
  base::ObserverList<ContentObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(ContentWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_CONTENT_WINDOW_H_
