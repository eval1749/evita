// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MOCK_MEDIA_H_
#define EVITA_VISUALS_CSS_MOCK_MEDIA_H_

#include "base/macros.h"
#include "evita/gfx/base/geometry/float_size.h"
#include "evita/visuals/css/media.h"

namespace visuals {
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
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MOCK_MEDIA_H_
