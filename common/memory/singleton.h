// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_memory_singleton_h)
#define INCLUDE_common_memory_singleton_h

namespace common {

template<class T>
class Singleton {
  public: static T* instance() {
    static T* instance;
    if (!instance)
      instance = new T();
    return instance;
  }
};

} // namespace common

#endif //!defined(INCLUDE_common_memory_singleton_h)
