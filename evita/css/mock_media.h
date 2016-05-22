// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_MOCK_MEDIA_H_
#define EVITA_CSS_MOCK_MEDIA_H_

#include "base/macros.h"
#include "evita/css/media.h"
#include "evita/gfx/base/geometry/float_size.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// MockMedia
//
class MockMedia : public Media {
 public:
  MockMedia();
  ~MockMedia();

  MediaState media_state() const final;
  MediaType media_type() const final;
  gfx::FloatSize viewport_size() const final;

  void SetViewportSize(const gfx::FloatSize& new_viewport_size);

 private:
  gfx::FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(MockMedia);
};

}  // namespace css

#endif  // EVITA_CSS_MOCK_MEDIA_H_
