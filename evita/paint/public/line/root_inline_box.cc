// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/paint/public/line/root_inline_box.h"

#include "base/logging.h"
#include "evita/layout/line/inline_box.h"

namespace paint {

// TODO(eval1749): We should use |paint::InlineBox| instead of
// |layout::InlineBox|.
using InlineBox = layout::InlineBox;

namespace {
std::vector<InlineBox*> CopyInlineBoxList(const std::vector<InlineBox*> boxes) {
  std::vector<InlineBox*> copy;
  copy.reserve(boxes.size());
  for (auto const& box : boxes)
    copy.push_back(box->Copy());
  return std::move(copy);
}
}  // namespace

RootInlineBox::RootInlineBox(const std::vector<InlineBox*>& boxes,
                             const gfx::RectF& bounds)
    : bounds_(bounds), boxes_(boxes), hash_code_(0) {}

RootInlineBox::RootInlineBox(const RootInlineBox& other)
    : bounds_(other.bounds_),
      boxes_(CopyInlineBoxList(other.boxes_)),
      hash_code_(other.hash_code_) {}

RootInlineBox::~RootInlineBox() {}

size_t RootInlineBox::ComputeHashCode() const {
  DCHECK(!boxes_.empty());
  if (hash_code_)
    return hash_code_;
  for (const auto box : boxes_) {
    hash_code_ <<= 5;
    hash_code_ ^= box->Hash();
    hash_code_ >>= 3;
  }
  return hash_code_;
}

RootInlineBox* RootInlineBox::Copy() const {
  DCHECK(!boxes_.empty());
  return new RootInlineBox(*this);
}

bool RootInlineBox::Equal(const RootInlineBox* other) const {
  DCHECK(!boxes_.empty());
  if (ComputeHashCode() != other->ComputeHashCode())
    return false;
  if (boxes_.size() != other->boxes_.size())
    return false;
  auto other_it = other->boxes_.begin();
  for (auto box : boxes_) {
    if (!box->Equal(*other_it))
      return false;
    ++other_it;
  }
  return true;
}

}  // namespace paint
