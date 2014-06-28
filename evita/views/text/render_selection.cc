// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_selection.h"

#include "base/logging.h"

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// TextSelectionModel
//
TextSelectionModel::TextSelectionModel(text::Posn start, text::Posn end,
                                       Active active)
    : active_(active), end_(end), start_(start) {
}

TextSelectionModel::TextSelectionModel(const TextSelectionModel& other)
    : TextSelectionModel(other.start_, other.end_, other.active_) {
}

TextSelectionModel::TextSelectionModel()
    : TextSelectionModel(0, 0, Active::NotActive) {
}

TextSelectionModel::~TextSelectionModel() {
}

bool TextSelectionModel::operator==(const TextSelectionModel& other) const {
  return active_ == other.active_ && end_ == other.end_ &&
         start_ == other.start_;
}

bool TextSelectionModel::operator!=(const TextSelectionModel& other) const {
  return !operator==(other);
}

text::Posn TextSelectionModel::active_offset() const {
  DCHECK(has_caret());
  return active_ == Active::StartIsActive ? start_ : end_;
}

bool TextSelectionModel::has_caret() const {
  return active_ == Active::StartIsActive || active_ == Active::EndIsActive;
}

//////////////////////////////////////////////////////////////////////
//
// TextSelection
//
TextSelection::TextSelection(const TextSelectionModel& model,
                             const gfx::ColorF& color)
    : color_(color), model_(model) {
}

TextSelection::TextSelection(const TextSelection& other)
    : TextSelection(other.model_, other.color_) {
}

TextSelection::TextSelection()
    : TextSelection(TextSelectionModel(), gfx::ColorF()) {
}

TextSelection::~TextSelection() {
}

bool TextSelection::operator==(const TextSelection& other) const {
  return model_ == other.model_ && color_ == other.color_;
}

bool TextSelection::operator!=(const TextSelection& other) const {
  return !operator==(other);
}

}  // rendering
}  // views
