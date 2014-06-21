// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_selection.h"

namespace views {
namespace rendering {

TextSelection::TextSelection()
    : TextSelection(gfx::ColorF(), 0, 0) {
}

TextSelection::TextSelection(const gfx::ColorF& color,
                             text::Posn start, text::Posn end)
    : color(color), start(start), end(end) {
}

TextSelection::~TextSelection() {
}

bool TextSelection::operator==(const TextSelection& other) const {
  return color == other.color && end == other.end && start == other.start;
}

bool TextSelection::operator!=(const TextSelection& other) const {
  return !operator==(other);
}

}  // rendering
}  // views
