// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_content_window_h)
#define INCLUDE_evita_views_content_window_h

#include <memory>

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/views/window.h"

class Frame;

namespace ui {
class MouseEvent;
}

namespace views {

class ContentObserver;

class ContentWindow : public Window {
  DECLARE_CASTABLE_CLASS(ContentWindow, Window);

  private: ObserverList<ContentObserver> observers_;

  protected: explicit ContentWindow(views::WindowId window_id);
  public: virtual ~ContentWindow();

  public: void Activate();
  public: void AddObserver(ContentObserver* observer);
  private: Frame* GetFrame() const;
  public: bool IsActive() const;
  public: virtual void MakeSelectionVisible() = 0;
  protected: void NotifyUpdateContent();
  public: void RemoveObserver(ContentObserver* observer);

  // ui::Widget
  protected: virtual void DidChangeBounds() override;
  protected: virtual void DidChangeHierarchy() override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidSetFocus(ui::Widget*) override;

  DISALLOW_COPY_AND_ASSIGN(ContentWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_content_window_h)
