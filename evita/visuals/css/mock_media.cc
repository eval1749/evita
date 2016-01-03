// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/mock_media.h"

#include "evita/visuals/css/media_type.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// MockMedia
//
MockMedia::MockMedia() : viewport_size_(320, 240) {}
MockMedia::~MockMedia() {}

MediaType MockMedia::media_type() const {
  return MediaType::Screen;
}

FloatSize MockMedia::viewport_size() const {
  return viewport_size_;
}

void MockMedia::SetViewportSize(const FloatSize& new_viewport_size) {
  if (viewport_size_ == new_viewport_size)
    return;
  viewport_size_ = new_viewport_size;
  DidChangeViewportSize();
}

}  // namespace css
}  // namespace visuals
