// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_RESOURCE_STRING_LIST_H_
#define EVITA_BASE_RESOURCE_STRING_LIST_H_

#include <stdint.h>

#include <iterator>
#include <utility>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/base/evita_base_export.h"

namespace base {
namespace resource {

//////////////////////////////////////////////////////////////////////
//
// StringList represents string list in resource pack.
//
class EVITA_BASE_EXPORT StringList {
 public:
  struct Entry {
    uint32_t offset;
  };

  class EVITA_BASE_EXPORT Iterator
      : public std::iterator<std::forward_iterator_tag, base::StringPiece> {
   public:
    Iterator(const StringList* list, const Entry* current);
    Iterator(const Iterator& other);
    ~Iterator();

    value_type operator*() const;

    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    const Entry* current_;
    const StringList* list_;
  };

  explicit StringList(base::StringPiece raw_list);
  ~StringList();

  Iterator begin() const { return Iterator(this, begin_entry()); }
  Iterator end() const { return Iterator(this, end_entry()); }
  size_t size() const { return *reinterpret_cast<const uint32_t*>(start_); }

 private:
  const Entry* begin_entry() const {
    return reinterpret_cast<const Entry*>(start_ + sizeof(uint32_t));
  }

  const Entry* end_entry() const { return begin_entry() + size(); }

  const uint8_t* const start_;

  DISALLOW_COPY_AND_ASSIGN(StringList);
};

//////////////////////////////////////////////////////////////////////
//
// StringPairList
//
class EVITA_BASE_EXPORT StringPairList {
 public:
  class EVITA_BASE_EXPORT Iterator
      : public std::iterator<std::forward_iterator_tag,
                             std::pair<base::StringPiece, base::StringPiece>> {
   public:
    explicit Iterator(const StringList::Iterator& current)
        : current_(current) {}
    Iterator(const Iterator& other) : Iterator(other.current_) {}
    ~Iterator();

    value_type operator*() const {
      return std::make_pair(*current_, *std::next(current_));
    }

    Iterator& operator++() {
      ++current_;
      ++current_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return current_ == other.current_;
    }

    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    StringList::Iterator current_;
  };

  explicit StringPairList(base::StringPiece raw_list);
  ~StringPairList();

  Iterator begin() const { return Iterator(list_.begin()); }
  Iterator end() const { return Iterator(list_.end()); }
  size_t size() const { return list_.size() / 2; }

 private:
  const StringList list_;

  DISALLOW_COPY_AND_ASSIGN(StringPairList);
};

}  // namespace resource
}  // namespace base

#endif  // EVITA_BASE_RESOURCE_STRING_LIST_H_
