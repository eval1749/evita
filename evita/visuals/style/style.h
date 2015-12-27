// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_H_
#define EVITA_VISUALS_STYLE_STYLE_H_

#include <iosfwd>
#include <unordered_set>

#include "evita/visuals/style/background.h"
#include "evita/visuals/style/border.h"
#include "evita/visuals/style/float_color.h"
#include "evita/visuals/style/padding.h"
#include "evita/visuals/style/margin.h"
#include "evita/visuals/style/style_properties_forward.h"

namespace visuals {

class StyleBuilder;
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
  type name() const;              \
  bool has_##name() const;

  FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V

 private:
  friend class StyleBuilder;
  friend class StyleEditor;

#define V(Name, name, type, text) type name##_;
  FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V
  std::unordered_set<StylePropertyId> contains_;
  int dummy_;
};

std::ostream& operator<<(std::ostream& ostream, const Style& style);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_H_
