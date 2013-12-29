// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_ref_counted_h)
#define INCLUDE_common_ref_counted_h

#include "base/basictypes.h"
#include "base/logging.h"

namespace common {

template<class T>
class RefCounted {
  private: mutable int ref_count_;

  protected: RefCounted()
    : ref_count_(0) {
    }

  protected: ~RefCounted() {
    DCHECK(!ref_count_);
  }

  public: void AddRef() const {
    ++ref_count_;
  }

  public: void Release() const {
    DCHECK_GE(ref_count_, 1);
    --ref_count_;
    if (!ref_count_)
      delete static_cast<T*>(const_cast<RefCounted*>(this));
  }

  DISALLOW_COPY_AND_ASSIGN(RefCounted);
};

}  // namespace common

#endif //!defined(INCLUDE_common_ref_counted_h)
