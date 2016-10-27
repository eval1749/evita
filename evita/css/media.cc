// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/media.h"

#include "evita/css/media_observer.h"
#include "evita/css/media_state.h"

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
  for (auto& observer : observers_)
    observer.DidChangeMediaState();
}

void Media::DidChangeSystemMetrics() {
  for (auto& observer : observers_)
    observer.DidChangeSystemMetrics();
}

void Media::DidChangeViewportSize() {
  for (auto& observer : observers_)
    observer.DidChangeViewportSize();
}

}  // namespace css
