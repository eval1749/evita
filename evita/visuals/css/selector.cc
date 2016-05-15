// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>

#include "evita/visuals/css/selector.h"

namespace visuals {
namespace css {

Selector::Selector(const Selector& other)
    : classes_(other.classes_), id_(other.id_), tag_name_(other.tag_name_) {}

Selector::Selector(Selector&& other)
    : classes_(std::move(other.classes_)),
      id_(other.id_),
      tag_name_(other.tag_name_) {}

Selector::Selector() = default;
Selector::~Selector() = default;

Selector& Selector::operator=(const Selector& other) {
  classes_ = other.classes_;
  id_ = other.id_;
  tag_name_ = other.tag_name_;
  return *this;
}

Selector& Selector::operator=(Selector&& other) {
  classes_ = std::move(other.classes_);
  id_ = other.id_;
  tag_name_ = other.tag_name_;
  other.id_ = base::AtomicString();
  other.tag_name_ = base::AtomicString();
  return *this;
}

bool Selector::operator==(const Selector& other) const {
  if (tag_name_ != other.tag_name_)
    return false;
  if (id_ != other.id_)
    return false;
  auto other_classes = other.classes_.begin();
  for (auto element : classes_) {
    if (other_classes == other.classes_.end())
      return false;
    auto other_element = *other_classes;
    if (element != other_element) {
      return false;
    }
    ++other_classes;
  }
  return other_classes == other.classes_.end();
}

bool Selector::operator!=(const Selector& other) const {
  return !operator==(other);
}

// Returns true if this |classes_| is longer than |other|'s.
bool Selector::operator<(const Selector& other) const {
  if (tag_name_ != other.tag_name_)
    return tag_name_ > other.tag_name_;
  if (id_ != other.id_)
    return id_ > other.id_;
  auto other_classes = other.classes_.begin();
  for (auto element : classes_) {
    if (other_classes == other.classes_.end())
      return true;
    auto other_element = *other_classes;
    if (element != other_element) {
      return element > other_element;
    }
    ++other_classes;
  }
  // |classes_| is equal to or shorter than |other.classes_|.
  return false;
}

bool Selector::IsSubsetOf(const Selector& other) const {
  if (tag_name_ != other.tag_name_ && !other.tag_name_.empty())
    return false;
  if (id_ != other.id_ && !other.id_.empty())
    return false;
  return std::includes(classes_.begin(), classes_.end(), other.classes_.begin(),
                       other.classes_.end());
}

std::ostream& operator<<(std::ostream& ostream, const Selector& selector) {
  if (!selector.tag_name().empty())
    ostream << selector.tag_name();
  if (!selector.id().empty())
    ostream << "#" << selector.id();
  for (auto class_name : selector.classes()) {
    if (class_name.value()[0] != ':')
      ostream << '.';
    ostream << class_name;
  }
  return ostream;
}

}  // namespace css
}  // namespace visuals
