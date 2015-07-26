// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_MEMORY_SINGLETON_H_
#define COMMON_MEMORY_SINGLETON_H_

namespace common {

template <class T>
class Singleton {
 public:
  Singleton() = default;
  virtual ~Singleton() = default;

  static T* instance() {
    static T* instance;
    if (!instance)
      instance = new T();
    return instance;
  }
};

#define DECLARE_SINGLETON_CLASS(name) friend class common::Singleton<name>

}  // namespace common

#endif  // COMMON_MEMORY_SINGLETON_H_
