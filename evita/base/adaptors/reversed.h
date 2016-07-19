// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_ADAPTORS_REVERSED_H_
#define EVITA_BASE_ADAPTORS_REVERSED_H_

#include <iterator>

#include "base/macros.h"

namespace base {

namespace internal {

// TODO(eval1749): Since "//base" version takes reference instead of const
// reference, it does not work with |common::Node::childNode()|. Once, "//base"
// version to take const reference, we should use "//base" version.

//////////////////////////////////////////////////////////////////////
//
// ReversedAdaptor
//
template <typename T>
class ReversedAdaptor {
 public:
  using iterator = typename T::reverse_iterator;

  explicit ReversedAdaptor(const T& x)
      : begin_(const_cast<T&>(x).rbegin()), end_(const_cast<T&>(x).rend()) {}

  ~ReversedAdaptor() = default;

  iterator begin() { return begin_; }
  iterator end() { return end_; }

 private:
  iterator begin_;
  iterator end_;

  DISALLOW_ASSIGN(ReversedAdaptor);
};

//////////////////////////////////////////////////////////////////////
//
// ConstReversedAdaptor
//
template <typename T>
class ConstReversedAdaptor {
 public:
  using const_iterator = typename T::const_reverse_iterator;

  explicit ConstReversedAdaptor(const T& x)
      : begin_(x.rbegin()), end_(x.rend()) {}

  ~ConstReversedAdaptor() = default;

  const_iterator begin() const { return begin_; }
  const_iterator end() const { return end_; }

 private:
  const_iterator begin_;
  const_iterator end_;

  DISALLOW_ASSIGN(ConstReversedAdaptor);
};

}  // namespace internal

template <typename T>
internal::ReversedAdaptor<T> Reversed(const T& x) {
  return internal::ReversedAdaptor<T>(x);
}

template <typename T>
internal::ConstReversedAdaptor<T> ConstReversed(const T& x) {
  return internal::ConstReverseAdaptor<T>(x);
}

}  // namespace base

#endif  // EVITA_BASE_ADAPTORS_REVERSED_H_
