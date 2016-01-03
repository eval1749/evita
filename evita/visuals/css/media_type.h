// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MEDIA_TYPE_H_
#define EVITA_VISUALS_CSS_MEDIA_TYPE_H_

#include <iosfwd>

namespace visuals {
namespace css {

#define FOR_EACH_VISUALS_CSS_MEDIA_TYPE(V) \
  V(Print)                                 \
  V(Screen)

//////////////////////////////////////////////////////////////////////
//
// MediaType
//
enum class MediaType {
#define V(name) name,
  FOR_EACH_VISUALS_CSS_MEDIA_TYPE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, MediaType media_type);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MEDIA_TYPE_H_
