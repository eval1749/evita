// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_selection.h"

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// TextSelectionModel
//
TextSelectionModel::TextSelectionModel(text::Posn start, text::Posn end,
                                       bool active)
    : active(active), start(start), end(end) {
}

TextSelectionModel::TextSelectionModel(const TextSelectionModel& other)
    : TextSelectionModel(other.start, other.end, other.active) {
}

TextSelectionModel::TextSelectionModel()
    : TextSelectionModel(0, 0, false) {
}

TextSelectionModel::~TextSelectionModel() {
}

bool TextSelectionModel::operator==(const TextSelectionModel& other) const {
  return active == other.active && end == other.end && start == other.start;
}

bool TextSelectionModel::operator!=(const TextSelectionModel& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(const TextSelectionModel& model,
                             const gfx::ColorF& color)
    : TextSelectionModel(model), color(color) {
}

TextSelection::TextSelection()
    : TextSelection(TextSelectionModel(), gfx::ColorF()) {
}

TextSelection::~TextSelection() {
}

bool TextSelection::operator==(const TextSelection& other) const {
  return TextSelectionModel::operator==(other) && color == other.color;
}

bool TextSelection::operator!=(const TextSelection& other) const {
  return !operator==(other);
}

}  // rendering
}  // views
