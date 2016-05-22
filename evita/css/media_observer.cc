// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/media_observer.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// MediaObserver
//
MediaObserver::MediaObserver() {}
MediaObserver::~MediaObserver() {}

void MediaObserver::DidChangeMediaState() {}
void MediaObserver::DidChangeSystemMetrics() {}
void MediaObserver::DidChangeViewportSize() {}

}  // namespace css
