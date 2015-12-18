// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/paint/public/line/root_inline_box.h"

#include "base/logging.h"
#include "evita/paint/public/line/inline_box.h"

namespace paint {

namespace {
std::vector<InlineBox*> CopyInlineBoxList(const std::vector<InlineBox*> boxes) {
  std::vector<InlineBox*> copy;
  copy.reserve(boxes.size());
  for (auto const& box : boxes)
    copy.push_back(box->Copy());
  return std::move(copy);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RootInlineBox
//
RootInlineBox::RootInlineBox(const std::vector<InlineBox*>& boxes,
                             const gfx::RectF& bounds)
    : bounds_(bounds), boxes_(boxes) {
  DCHECK(!boxes.empty());
  DCHECK(!boxes_.empty());
}

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
  auto const copy = new RootInlineBox(CopyInlineBoxList(boxes_), bounds_);
  copy->hash_code_ = hash_code_;
  return copy;
}

bool RootInlineBox::Equal(const RootInlineBox* other) const {
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
