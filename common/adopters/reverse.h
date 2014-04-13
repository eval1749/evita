// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_adopters_reverse_h)
#define INCLUDE_common_adopters_reverse_h

//#include <utility>

namespace common {
namespace adopters {

namespace internal {

template <typename T>
class Reverse {
  public: typedef typename T::reverse_iterator iterator;

  private: iterator begin_;
  private: iterator end_;

  public: Reverse(const T& x)
      : begin_(const_cast<T&>(x).rbegin()),
        end_(const_cast<T&>(x).rend()) {
  }
  public: ~Reverse() {
  }

  public: iterator begin() { return begin_; }
  public: iterator end() { return end_; }
};

template <typename T>
class ConstReverse {
  public: typedef typename T::const_reverse_iterator const_iterator;

  private: const_iterator begin_;
  private: const_iterator end_;

  public: ConstReverse(const T& x) : begin_(x.rbegin()), end_(x.rend()) {
  }

  public: ~ConstReverse() {
  }

  public: const_iterator begin() const { return begin_; }
  public: const_iterator end() const { return end_; }
};
} // namespace internal

template <typename T>
internal::Reverse<T> reverse(const T& x) {
  return internal::Reverse<T>(x);
}

template <typename T>
internal::ConstReverse<T> const_reverse(const T& x) {
  return internal::ConstReverse<T>(x);
}

} // namespace adopters
} // namespace common

#endif //!defined(INCLUDE_common_adopters_reverse_h)
