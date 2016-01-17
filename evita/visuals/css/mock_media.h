// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MOCK_MEDIA_H_
#define EVITA_VISUALS_CSS_MOCK_MEDIA_H_

#include "base/macros.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/geometry/float_size.h"

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

  bool is_caret_on() const final;
  MediaState media_state() const final;
  MediaType media_type() const final;
  FloatSize viewport_size() const final;

  void SetViewportSize(const FloatSize& new_viewport_size);

 private:
  FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(MockMedia);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MOCK_MEDIA_H_
