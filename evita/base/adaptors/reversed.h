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
struct ReversedAdaptor {
  T& iterable;
};

//////////////////////////////////////////////////////////////////////
//
// ConstReversedAdaptor
//
template <typename T>
struct ConstReversedAdaptor {
  const T& iterable;
};

}  // namespace internal

template <typename T>
internal::ReversedAdaptor<T> Reversed(T&& thing) {
  return {thing};
}

template <typename T>
internal::ConstReversedAdaptor<T> ConstReversed(const T& thing) {
  return {thing};
}

}  // namespace base

namespace std {

template <typename T>
auto begin(base::internal::ReversedAdaptor<T> x) {
  return rbegin(x.iterable);
}

template <typename T>
auto end(base::internal::ReversedAdaptor<T> x) {
  return rend(x.iterable);
}

template <typename T>
auto begin(base::internal::ConstReversedAdaptor<T> x) {
  return rbegin(x.iterable);
}

template <typename T>
auto end(base::internal::ConstReversedAdaptor<T> x) {
  return rend(x.iterable);
}

}  // namespace std

#endif  // EVITA_BASE_ADAPTORS_REVERSED_H_
