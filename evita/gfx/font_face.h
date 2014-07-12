// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_font_face_h)
#define INCLUDE_evita_gfx_font_face_h

#include <functional>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FontProperties
//
struct FontProperties {
  bool bold;
  base::string16 family_name;
  float font_size_pt;
  bool italic;

  FontProperties();
  bool operator==(const FontProperties& other) const;
  bool operator!=(const FontProperties& other) const;
};

//////////////////////////////////////////////////////////////////////
//
// FontFace
//
class FontFace : public SimpleObject_<IDWriteFontFace> {
  private: const DWRITE_FONT_METRICS metrics_;
  public: FontFace(const FontProperties& properties);
  public: ~FontFace();
  public: const DWRITE_FONT_METRICS& metrics() const { return metrics_; }
  DISALLOW_COPY_AND_ASSIGN(FontFace);
};

} // namespace gfx

namespace std {
template<> struct hash<gfx::FontProperties> {
  size_t operator()(const gfx::FontProperties& properties) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_gfx_font_face_h)
