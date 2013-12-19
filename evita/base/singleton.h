// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_singleton_h)
#define INCLUDE_base_singleton_h

namespace base {

template<class T>
class Singleton {
  public: static T& instance() {
    static T* instance;
    if (!instance)
      instance = new T();
    return *instance;
  }
};

} // namespace base

#endif //!defined(INCLUDE_base_singleton_h)
