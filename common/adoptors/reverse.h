// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_adoptors_reverse_h)
#define INCLUDE_common_adoptors_reverse_h

#include <utility>

namespace common {
namespace adoptors {

namespace impl {
template<class Container>
class ConstReverser {
  private: Container& container_;
  public: explicit ConstReverser(Container& container)
      : container_(container) {
  }
  public: ConstReverser(const ConstReverser& other)
      : container_(other.container_) {
  }
  public: ConstReverser& operator=(const ConstReverser&) = delete;
  public: auto begin() -> decltype(container_.crbegin()) {
    return container_.crbegin();
  }
  public: auto end() -> decltype(container_.crend()) {
    return container_.crend();
  }
};

template<class Container>
class Reverser {
  private: Container& container_;
  public: explicit Reverser(Container& container) : container_(container) {
  }
  public: Reverser(const Reverser& other)
      : container_(other.container_) {
  }
  public: Reverser& operator=(const Reverser&) = delete;
  public: auto begin() -> decltype(container_.rbegin()) {
    return container_.rbegin();
  }
  public: auto end() -> decltype(container_.rend()) {
    return container_.rend();
  }
};
} // namespace impl

template<class Container>
impl::ConstReverser<Container> reverse(const Container& container) {
  return impl::ConstReverser<Container>(const_cast<Container&>(container));
}

template<class Container>
impl::Reverser<Container> reverse(Container& container) {
  return impl::Reverser<Container>(container);
}

} // namespace adoptors
} // namespace common

#endif //!defined(INCLUDE_common_adoptors_reverse_h)
