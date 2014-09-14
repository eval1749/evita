// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_content_observer_h)
#define INCLUDE_evita_views_content_observer_h

#include "base/basictypes.h"

namespace views {

class ContentWindow;

class ContentObserver {
  protected: ContentObserver();
  public: virtual ~ContentObserver();

  public: virtual void DidUpdateContent(ContentWindow* content_window) = 0;

  DISALLOW_COPY_AND_ASSIGN(ContentObserver);
};

}   // namespace views

#endif //!defined(INCLUDE_evita_views_content_observer_h)
