// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MEDIA_OBSERVER_H_
#define EVITA_VISUALS_CSS_MEDIA_OBSERVER_H_

#include "base/macros.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// MediaObserver
//
class MediaObserver {
 public:
  ~MediaObserver();

  virtual void DidChangeMediaState();
  virtual void DidChangeSystemMetrics();
  virtual void DidChangeViewportSize();

 protected:
  MediaObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(MediaObserver);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MEDIA_OBSERVER_H_
