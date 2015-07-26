// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef COMMON_ADOPTERS_REVERSE_H_
#define COMMON_ADOPTERS_REVERSE_H_

namespace common {
namespace adopters {

namespace internal {

template <typename T>
class Reverse {
 public:
  typedef typename T::reverse_iterator iterator;

  explicit Reverse(const T& x)
      : begin_(const_cast<T&>(x).rbegin()), end_(const_cast<T&>(x).rend()) {}
  ~Reverse() {}

  iterator begin() { return begin_; }
  iterator end() { return end_; }

 private:
  iterator begin_;
  iterator end_;
};

template <typename T>
class ConstReverse {
 public:
  typedef typename T::const_reverse_iterator const_iterator;

  explicit ConstReverse(const T& x) : begin_(x.rbegin()), end_(x.rend()) {}
  ~ConstReverse() {}

  const_iterator begin() const { return begin_; }
  const_iterator end() const { return end_; }

 private:
  const_iterator begin_;
  const_iterator end_;
};

}  // namespace internal

template <typename T>
internal::Reverse<T> reverse(const T& x) {
  return internal::Reverse<T>(x);
}

template <typename T>
internal::ConstReverse<T> const_reverse(const T& x) {
  return internal::ConstReverse<T>(x);
}

}  // namespace adopters
}  // namespace common

#endif  // COMMON_ADOPTERS_REVERSE_H_
