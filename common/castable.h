// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_CASTABLE_H_
#define COMMON_CASTABLE_H_

#include <type_traits>

namespace common {

// T* as()
// const char* class_name()
// bool is<T>()
// static const char* static_class_name()
template <typename Base>
class Castable {
 public:
  template <class Class>
  Class* as() {
    return is<Class>() ? static_cast<Class*>(this) : nullptr;
  }

  template <class Class>
  const Class* as() const {
    return is<Class>() ? static_cast<const Class*>(this) : nullptr;
  }

  virtual const char* class_name() const { return static_class_name(); }

  template <class Class>
  bool is() const {
    static_assert(std::is_base_of<Base, Class>::value, "Unrelated classes");
    return is_class_of(Class::static_class_name());
  }

 protected:
  Castable() = default;
  virtual ~Castable() = default;

  virtual bool is_class_of(const char* other_name) const {
    return static_class_name() == other_name;
  }

 public:
  static const char* static_class_name() { return "Castable"; }
};

#define DECLARE_CASTABLE_CLASS(this_name, base_name)                      \
 public:                                                                  \
  static const char* static_class_name() { return #this_name; }           \
                                                                          \
  const char* class_name() const override { return static_class_name(); } \
                                                                          \
 protected:                                                               \
  bool is_class_of(const char* other_name) const override {               \
    return static_class_name() == other_name ||                           \
           base_name::is_class_of(other_name);                            \
  }

}  // namespace common

#endif  // COMMON_CASTABLE_H_
