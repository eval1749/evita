// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_CONTENT_OBSERVER_H_
#define EVITA_VIEWS_CONTENT_OBSERVER_H_

#include "base/macros.h"

namespace views {

class ContentWindow;

class ContentObserver {
 public:
  virtual ~ContentObserver();

  virtual void DidActivateContent(ContentWindow* content_window) = 0;
  virtual void DidUpdateContent(ContentWindow* content_window) = 0;

 protected:
  ContentObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(ContentObserver);
};

}   // namespace views

#endif  // EVITA_VIEWS_CONTENT_OBSERVER_H_
