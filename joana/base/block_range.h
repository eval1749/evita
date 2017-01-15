// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_BLOCK_RANGE_H_
#define JOANA_BASE_BLOCK_RANGE_H_

#include <iterator>

#include "base/logging.h"

namespace joana {

//
// BlockRange
//
template <typename Element>
class BlockRange final {
 public:
  //
  // Iterator
  //
  class Iterator final
      : public std::iterator<std::input_iterator_tag, Element> {
   public:
    Iterator(const Iterator& other) : Iterator(other.owner_, other.index_) {}

    auto& operator*() const {
      DCHECK_LT(index_, owner_->size_);
      return *owner_->elements_[index_];
    }

    Iterator& operator++() {
      DCHECK_LT(index_, owner_->size_);
      ++index_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      DCHECK_EQ(owner_, other.owner_);
      return index_ == other.index_;
    }

    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    friend class BlockRange;

    Iterator(const BlockRange* owner, size_t index)
        : index_(index), owner_(owner) {
      DCHECK_LE(index_, owner_->size_);
    }

    size_t index_;
    const BlockRange* owner_;
  };

  BlockRange(const Element* elements, size_t size)
      : elements_(elements), size_(size) {}

  BlockRange(const BlockRange& other)
      : BlockRange(other.elements_, other.size_) {}

  ~BlockRange() = default;

  Iterator begin() const { return Iterator(this, 0); }
  bool empty() const { return size_ == 0; }
  Iterator end() const { return Iterator(this, size_); }
  size_t size() const { return size_; }

 private:
  const Element* elements_;
  size_t size_;
};

}  // namespace joana

#endif  // JOANA_BASE_BLOCK_RANGE_H_
