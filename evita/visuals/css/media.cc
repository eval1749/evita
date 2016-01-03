// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/media.h"

#include "evita/visuals/css/media_observer.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Media
//
Media::Media() {}
Media::~Media() {}

void Media::AddObserver(MediaObserver* observer) const {
  observers_.AddObserver(observer);
}

void Media::RemoveObsever(MediaObserver* observer) const {
  observers_.AddObserver(observer);
}

void Media::DidChangeViewportSize() {
  FOR_EACH_OBSERVER(MediaObserver, observers_, DidChangeViewportSize());
}

void Media::DidChangeSystemMetrics() {
  FOR_EACH_OBSERVER(MediaObserver, observers_, DidChangeSystemMetrics());
}

}  // namespace css
}  // namespace visuals
