// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_HASH_UTILS_H_
#define JOANA_BASE_HASH_UTILS_H_

#include <functional>
#include <tuple>
#include <utility>

#include "base/strings/string_piece.h"

// Provide
//  HashCombine()
//  std::hash for std::pair<First, Second>
//  std::hash for std::tuple<Members...>

namespace std {

template <>
struct hash<base::StringPiece> {
  size_t operator()(base::StringPiece piece) const {
    return base::StringPieceHash()(piece);
  }
};

template <>
struct hash<base::StringPiece16> {
  size_t operator()(base::StringPiece16 piece) const {
    return base::StringPiece16Hash()(piece);
  }
};

}  // namespace std

namespace joana {

template <class T>
constexpr inline size_t HashCombine(size_t seed, const T& v) {
  return seed ^ std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace internal {

template <typename Tuple, size_t I, size_t... Is>
constexpr size_t HashTupleInternal(size_t seed,
                                   const Tuple& tuple,
                                   std::index_sequence<I, Is...>) {
  return HashCombine(
      HashCombine(seed, std::get<sizeof...(Is)>(tuple)),
      HashTupleInternal(seed, tuple,
                        std::make_index_sequence<sizeof...(Is)>()));
}

template <typename Tuple>
constexpr size_t HashTupleInternal(size_t seed,
                                   const Tuple& tuple,
                                   std::index_sequence<>) {
  return seed;
}

// Compute hash value of |tuple|.
template <typename... Members>
constexpr size_t HashTuple(const std::tuple<Members...>& tuple) {
  return HashTupleInternal(0, tuple, std::index_sequence_for<Members...>{});
}

}  // namespace internal

}  // namespace joana

namespace std {

// std::hash<std::tuple<Members...>>
template <typename... Members>
struct hash<std::tuple<Members...>> {
  size_t operator()(const std::tuple<Members...>& tuple) const {
    return joana::internal::HashTuple(tuple);
  }
};

// std::hash<std::pair<First, Second>
template <typename First, typename Second>
struct hash<std::pair<First, Second>> {
  size_t operator()(const std::pair<First, Second>& pair) const {
    return joana::HashCombine(std::hash<First>{}(pair.first),
                              std::hash<Second>{}(pair.second));
  }
};

}  // namespace std

#endif  // JOANA_BASE_HASH_UTILS_H_
