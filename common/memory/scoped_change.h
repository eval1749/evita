// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_memory_ScopedChange_h)
#define INCLUDE_common_memory_ScopedChange_h

namespace common {

template<typename T>
class ScopedChange {
  private: T original_value_;
  private: T& scoped_variable_;

  public: ScopedChange(T& scoped_variable, T new_value)
      : original_value_(scoped_variable), scoped_variable_(scoped_variable) {
    scoped_variable_ = new_value;
  }
  public: ~ScopedChange() {
    scoped_variable_ = original_value_;
  }

  DISALLOW_COPY_AND_ASSIGN(ScopedChange);

};

}  // namespace common

#endif //!defined(INCLUDE_common_memory_ScopedChange_h)
