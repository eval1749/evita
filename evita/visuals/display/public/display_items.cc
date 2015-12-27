// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/display/public/display_items.h"

namespace visuals {

namespace {
int last_display_item_id_;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DisplayItem
//
DisplayItem::DisplayItem() : id_(++last_display_item_id_) {}
DisplayItem::~DisplayItem() {}

bool DisplayItem::operator==(const DisplayItem& other) const {
  if (this == &other)
    return true;
  return EqualsTo(other);
}

bool DisplayItem::operator!=(const DisplayItem& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const DisplayItem& item) {
  return ostream << item.class_name() << '.' << item.id();
}

std::ostream& operator<<(std::ostream& ostream, const DisplayItem* item) {
  if (!item)
    return ostream << "nullptr";
  return ostream << *item;
}

//////////////////////////////////////////////////////////////////////
//
// BeginClipDisplayItem
//
BeginClipDisplayItem::BeginClipDisplayItem(const FloatRect& bounds)
    : bounds_(bounds) {}
BeginClipDisplayItem::~BeginClipDisplayItem() {}

bool BeginClipDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<BeginClipDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawRectDisplayItem
//
DrawRectDisplayItem::DrawRectDisplayItem(const FloatRect& bounds,
                                         const FloatColor& color,
                                         float thickness)
    : bounds_(bounds), color_(color), thickness_(thickness) {}
DrawRectDisplayItem::~DrawRectDisplayItem() {}

bool DrawRectDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawRectDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_ && color_ == item->color_ &&
         thickness_ == item->thickness_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawTextDisplayItem
//
DrawTextDisplayItem::DrawTextDisplayItem(const FloatRect& bounds,
                                         const FloatColor& color,
                                         float baseline,
                                         const base::string16& text)
    : baseline_(baseline), bounds_(bounds), color_(color), text_(text) {}
DrawTextDisplayItem::~DrawTextDisplayItem() {}

bool DrawTextDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawTextDisplayItem>();
  if (!item)
    return false;
  return baseline_ == item->baseline_ && bounds_ == item->bounds_ &&
         color_ == item->color_ && text_ == item->text_;
}

//////////////////////////////////////////////////////////////////////
//
// EndClipDisplayItem
//
EndClipDisplayItem::EndClipDisplayItem() {}
EndClipDisplayItem::~EndClipDisplayItem() {}

bool EndClipDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  return other.is<EndClipDisplayItem>();
}

//////////////////////////////////////////////////////////////////////
//
// FillRectDisplayItem
//
FillRectDisplayItem::FillRectDisplayItem(const FloatRect& bounds,
                                         const FloatColor& color)
    : bounds_(bounds), color_(color) {}
FillRectDisplayItem::~FillRectDisplayItem() {}

bool FillRectDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<FillRectDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_ && color_ == item->color_;
}

}  // namespace visuals
