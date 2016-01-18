// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/media.h"

#include "evita/visuals/css/media_observer.h"
#include "evita/visuals/css/media_state.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Media
//
Media::Media() {}
Media::~Media() {}

bool Media::is_media_active() const {
  return media_state() == MediaState::Active;
}

bool Media::is_media_inactive() const {
  return media_state() == MediaState::Inactive;
}

bool Media::is_media_interactive() const {
  return media_state() == MediaState::Interactive;
}

void Media::AddObserver(MediaObserver* observer) const {
  observers_.AddObserver(observer);
}

void Media::RemoveObserver(MediaObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void Media::DidChangeMediaState() {
  FOR_EACH_OBSERVER(MediaObserver, observers_, DidChangeMediaState());
}

void Media::DidChangeSystemMetrics() {
  FOR_EACH_OBSERVER(MediaObserver, observers_, DidChangeSystemMetrics());
}

void Media::DidChangeViewportSize() {
  FOR_EACH_OBSERVER(MediaObserver, observers_, DidChangeViewportSize());
}

}  // namespace css
}  // namespace visuals
