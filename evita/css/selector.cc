// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <vector>

#include "evita/css/selector.h"

#include "base/strings/string_util.h"

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

// Returns true |this| comes before |other| in lexicographical order.
// Example:
//  *
//  foo
//  foo#id1.c1.c2
//  foo#id1.c1
//  foo#id1.c2.c3
//  foo#id1.c2
//  bar
bool Selector::operator<(const Selector& other) const {
  // Check tag name
  if (tag_name_ != other.tag_name_)
    return tag_name_ < other.tag_name_;

  // Check id
  if (id_ != other.id_)
    return id_ < other.id_;

  // Check classes
  auto other_classes = other.classes_.begin();
  for (const auto& this_class : classes_) {
    if (other_classes == other.classes_.end()) {
      // |classes_| is longer than |other.classes_|.
      return true;
    }
    const auto& other_class = *other_classes;
    if (this_class != other_class)
      return this_class < other_class;
    ++other_classes;
  }
  // |classes_| equals to or shorter than |other.classes_|.
  return false;
}

bool Selector::IsMoreSpecific(const Selector& other) const {
  if (id_ != other.id_)
    return !other.has_id();
  if (classes_.size() != other.classes_.size())
    return classes_.size() > other.classes_.size();
  if (tag_name_ == other.tag_name_)
    return false;
  return other.is_universal();
}

bool Selector::IsSubsetOf(const Selector& other) const {
  if (tag_name_ != other.tag_name_ && !other.is_universal())
    return false;
  if (id_ != other.id_ && other.has_id())
    return false;
  return std::includes(classes_.begin(), classes_.end(), other.classes_.begin(),
                       other.classes_.end());
}

base::string16 Selector::ToString() const {
  if (*this == Selector())
    return L"*";
  std::vector<base::string16> strings;
  if (!tag_name_.empty())
    strings.emplace_back(tag_name_.value().as_string());
  if (!id_.empty()) {
    strings.emplace_back(L"#");
    strings.emplace_back(id_.value().as_string());
  }
  for (auto class_name : classes_) {
    if (class_name.value()[0] != ':')
      strings.emplace_back(L".");
    strings.emplace_back(class_name.value().as_string());
  }
  return base::JoinString(strings, base::StringPiece16());
}

std::ostream& operator<<(std::ostream& ostream, const Selector& selector) {
  return ostream << selector.ToString();
}

}  // namespace css
