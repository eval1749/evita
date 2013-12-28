// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_lock_h)
#define INCLUDE_evita_dom_lock_h

#include <memory>

#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"

namespace dom {

class Lock : public common::Singleton<Lock> {
  friend class common::Singleton<Lock>;

  private: std::unique_ptr<base::Lock> lock_;

  private: Lock();
  public: virtual ~Lock() = default;

  public: base::Lock* lock() const { return lock_.get(); }

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

}  // namespace dom

#define ASSERT_DOM_IS_LOCKED() \
  dom::Lock::instance().lock()->AssertAcquired()

#define DOM_AUTO_LOCK_SCOPE() \
  base::AutoLock dom_lock_scope(*dom::Lock::instance().lock());

#define DOM_AUTO_UNLOCK_SCOPE() \
  base::AutoUnlock dom_lock_scope(*dom::Lock::instance().lock());

#endif //!defined(INCLUDE_evita_dom_lock_h)
