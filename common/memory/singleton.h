// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_memory_singleton_h)
#define INCLUDE_common_memory_singleton_h

namespace common {

template<class T>
class Singleton {
  public: Singleton() = default;
  public: virtual ~Singleton() = default;

  public: static T* instance() {
    static T* instance;
    if (!instance)
      instance = new T();
    return instance;
  }
};

#define DECLARE_SINGLETON_CLASS(name) \
  friend class common::Singleton<name>

} // namespace common

#endif //!defined(INCLUDE_common_memory_singleton_h)
