// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_STYLE_H_
#define EVITA_VISUALS_CSS_STYLE_H_

#include <bitset>
#include <iosfwd>

#include "evita/visuals/css/background.h"
#include "evita/visuals/css/color.h"
#include "evita/visuals/css/properties_forward.h"
#include "evita/visuals/css/values.h"

namespace visuals {
namespace css {

class StyleEditor;

//////////////////////////////////////////////////////////////////////
//
// Style
//
class Style final {
 public:
  Style(const Style& other);
  Style();
  ~Style();

  bool operator==(const Style& other) const;
  bool operator!=(const Style& other) const;

#define V(Name, name, type, text) \
  const type& name() const;       \
  bool has_##name() const;

  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

 private:
  friend class StyleEditor;

#define V(Name, name, type, text) type name##_;
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
  std::bitset<kMaxPropertyId + 1> contains_;
};

std::ostream& operator<<(std::ostream& ostream, const Style& style);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_STYLE_H_
