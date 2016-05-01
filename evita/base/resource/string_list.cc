// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "evita/base/resource/string_list.h"

#include "base/logging.h"

namespace base {
namespace resource {

//////////////////////////////////////////////////////////////////////
//
// StirngList::Iterator
//
StringList::Iterator::Iterator(const StringList* list, const Entry* current)
    : current_(current), list_(list) {}

StringList::Iterator::Iterator(const Iterator& other)
    : Iterator(other.list_, other.current_) {}

StringList::Iterator::~Iterator() = default;

StringList::Iterator::value_type StringList::Iterator::operator*() const {
  DCHECK_LE(current_ + 1, list_->end_entry());
  const auto data =
      reinterpret_cast<const char*>(list_->start_ + current_->offset);
  auto const size = (current_ + 1)->offset - current_->offset;
  return base::StringPiece(data, size);
}

StringList::Iterator& StringList::Iterator::operator++() {
  DCHECK_LT(current_, list_->end_entry());
  ++current_;
  return *this;
}

bool StringList::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(list_, other.list_);
  return current_ == other.current_;
}

//////////////////////////////////////////////////////////////////////
//
// StirngList
//
StringList::StringList(base::StringPiece raw_list)
    : start_(reinterpret_cast<const uint8_t*>(raw_list.data())) {}

StringList::~StringList() = default;

//////////////////////////////////////////////////////////////////////
//
// StringPairList::Iterator
//
StringPairList::Iterator::~Iterator() = default;

//////////////////////////////////////////////////////////////////////
//
// StringPairList
//
StringPairList::StringPairList(base::StringPiece raw_list) : list_(raw_list) {
  DCHECK(list_.size() % 2 == 0) << list_.size() << " must be an even.";
}

StringPairList::~StringPairList() = default;

}  // namespace resource
}  // namespace base
