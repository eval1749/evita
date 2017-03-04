// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_ITERATOR_UTILS_H_
#define JOANA_BASE_ITERATOR_UTILS_H_

#include <iterator>

namespace joana {

namespace internal {

// Get range returning reference T& for std::vector<T*>.
template <typename BaseIterator>
class ReferenceRange final {
 public:
  class Iterator final
      : public std::iterator<std::input_iterator_tag,
                             typename BaseIterator::value_type> {
   public:
    Iterator(const Iterator& other) : it_(other.it_) {}
    ~Iterator() = default;

    auto& operator*() const { return **it_; }

    Iterator& operator++() {
      ++it_;
      return *this;
    }

    bool operator==(const Iterator& other) const { return it_ == other.it_; }
    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    friend class ReferenceRange;

    explicit Iterator(const BaseIterator& it) : it_(it) {}

    BaseIterator it_;
  };

  ReferenceRange(const BaseIterator& begin, const BaseIterator& end)
      : begin_(begin), end_(end) {}

  ReferenceRange(const ReferenceRange& other)
      : begin_(other.begin_), end_(other.end_) {}

  ~ReferenceRange() = default;

  Iterator begin() const { return Iterator(begin_); }
  bool empty() const { return begin_ == end_; }
  auto& front() const { return **begin_; }
  Iterator end() const { return Iterator(end_); }
  size_t size() const { return end_ - begin_; }

 private:
  BaseIterator begin_;
  BaseIterator end_;
};

}  //  namespace internal

template <typename Container>
internal::ReferenceRange<typename Container::iterator> ReferenceRangeOf(
    Container& container) {  // NOLINT
  using Iterator = typename Container::iterator;
  return internal::ReferenceRange<Iterator>(container.begin(), container.end());
}

template <typename Container>
internal::ReferenceRange<typename Container::const_iterator> ReferenceRangeOf(
    const Container& container) {
  using Iterator = typename Container::const_iterator;
  return internal::ReferenceRange<Iterator>(container.begin(), container.end());
}

}  // namespace joana

#endif  // JOANA_BASE_ITERATOR_UTILS_H_
