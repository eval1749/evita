// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef COMMON_MEMORY_SCOPED_CHANGE_H_
#define COMMON_MEMORY_SCOPED_CHANGE_H_

namespace common {

template <typename T>
class ScopedChange {
 public:
  ScopedChange(T& scoped_variable, T new_value)  // NOLINT
      : original_value_(scoped_variable),
        scoped_variable_(scoped_variable) {
    scoped_variable_ = new_value;
  }
  ~ScopedChange() { scoped_variable_ = original_value_; }

 private:
  T original_value_;
  T& scoped_variable_;

  DISALLOW_COPY_AND_ASSIGN(ScopedChange);
};

}  // namespace common

#endif  // COMMON_MEMORY_SCOPED_CHANGE_H_
