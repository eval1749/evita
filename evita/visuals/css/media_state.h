// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MEDIA_STATE_H_
#define EVITA_VISUALS_CSS_MEDIA_STATE_H_

#include <iosfwd>

namespace visuals {
namespace css {

#define FOR_EACH_CSS_MEDIA_STATE(V) \
  V(Active)                         \
  V(Inactive)                       \
  V(Interactive)

enum class MediaState {
#define V(name) name,
  FOR_EACH_CSS_MEDIA_STATE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, MediaState media_state);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MEDIA_STATE_H_
