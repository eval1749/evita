// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_MEMORY_REF_COUNTED_H_
#define COMMON_MEMORY_REF_COUNTED_H_

#include "base/basictypes.h"
#include "base/logging.h"

namespace common {

template <class T>
class RefCounted {
 public:
  void AddRef() const { ++ref_count_; }

  void Release() const {
    DCHECK_GE(ref_count_, 1);
    --ref_count_;
    if (!ref_count_)
      delete static_cast<T*>(const_cast<RefCounted*>(this));
  }

 protected:
  RefCounted() : ref_count_(0) {}
  ~RefCounted() { DCHECK(!ref_count_); }

 private:
  mutable int ref_count_;

  DISALLOW_COPY_AND_ASSIGN(RefCounted);
};

}  // namespace common

#endif  // COMMON_MEMORY_REF_COUNTED_H_
