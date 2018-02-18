// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MAYBE_H_
#define EVITA_BASE_MAYBE_H_

#include <utility>

#include "base/logging.h"

namespace base {

// A simple |Maybe| type, representing a value which may or may not have a
// value, see https://hackage.haskell.org/package/base/docs/Data-Maybe.html.
// This class is inspired by v8's |Maybe<T>| class.
template <typename T>
class Maybe {
 public:
  Maybe(const Maybe& other)
      : has_value_(other.has_value_),
        value_(other.has_value_ ? other.value_ : T()) {}

  Maybe(Maybe&& other)
      : has_value_(other.has_value_),
        value_(other.has_value_ ? std::move(other.value_) : T()) {}

  bool operator==(const Maybe& other) const;
  bool operator!=(const Maybe& other) const;

  T FromJust() const;
  T FromMaybe(const T& default_value) const;
  T MoveFromJust();

  bool IsJust() const { return has_value_; }
  bool IsNothing() const { return !has_value_; }

 private:
  template <typename U>
  friend Maybe<U> Just(const U& value);

  template <typename U>
  friend Maybe<U> Just(U&& value);

  template <typename U>
  friend Maybe<U> Nothing();

  explicit Maybe(const T& value) : has_value_(true), value_(value) {}
  explicit Maybe(T&& value) : has_value_(true), value_(std::move(value)) {}
  Maybe() = default;

  bool has_value_ = false;
  T value_;
};

template <typename T>
bool Maybe<T>::operator==(const Maybe& other) const {
  if (IsNothing())
    return other.IsNothing();
  if (other.IsNothing())
    return false;
  return FromJust() == other.FromJust();
}

template <typename T>
bool Maybe<T>::operator!=(const Maybe& other) const {
  return !operator==(other);
}

template <typename T>
T Maybe<T>::FromMaybe(const T& default_value) const {
  return IsJust() ? value_ : default_value;
}

template <typename T>
T Maybe<T>::MoveFromJust() {
  DCHECK(IsJust());
  return std::move(value_);
}

template <typename T>
T Maybe<T>::FromJust() const {
  DCHECK(IsJust());
  return value_;
}

// Constructors
template <typename T>
Maybe<T> Just(const T& value) {
  return Maybe<T>(value);
}

template <typename T>
Maybe<T> Just(T&& value) {
  return Maybe<T>(std::move(value));
}

template <typename T>
Maybe<T> Nothing() {
  return Maybe<T>();
}

}  // namespace base

#endif  // EVITA_BASE_MAYBE_H_
