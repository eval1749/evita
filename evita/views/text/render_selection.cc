// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_selection.h"

#include <algorithm>

#include "base/logging.h"

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// TextSelectionModel
//
TextSelectionModel::TextSelectionModel(State state,
                                       text::Posn anchor_offset,
                                       text::Posn focus_offset)
    : end_(std::max(anchor_offset, focus_offset)),
      focus_offset_(focus_offset),
      state_(state),
      start_(std::min(anchor_offset, focus_offset)) {}

TextSelectionModel::TextSelectionModel(const TextSelectionModel& other)
    : TextSelectionModel(other.state_,
                         other.anchor_offset(),
                         other.focus_offset_) {}

TextSelectionModel::TextSelectionModel()
    : TextSelectionModel(State::Disabled, 0, 0) {}

TextSelectionModel::~TextSelectionModel() {}

bool TextSelectionModel::operator==(const TextSelectionModel& other) const {
  return end_ == other.end_ && focus_offset_ == other.focus_offset_ &&
         state_ == other.state_ && start_ == other.start_;
}

bool TextSelectionModel::operator!=(const TextSelectionModel& other) const {
  return !operator==(other);
}

text::Posn TextSelectionModel::anchor_offset() const {
  return focus_offset_ == end_ ? start_ : end_;
}

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(const TextSelectionModel& model,
                             const gfx::ColorF& color)
    : color_(color), model_(model) {}

TextSelection::TextSelection(const TextSelection& other)
    : TextSelection(other.model_, other.color_) {}

TextSelection::TextSelection()
    : TextSelection(TextSelectionModel(), gfx::ColorF()) {}

TextSelection::~TextSelection() {}

bool TextSelection::operator==(const TextSelection& other) const {
  return model_ == other.model_ && color_ == other.color_;
}

bool TextSelection::operator!=(const TextSelection& other) const {
  return !operator==(other);
}

}  // namespace rendering
}  // namespace views
