// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_lock_h)
#define INCLUDE_evita_dom_lock_h

#include <memory>

#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"

namespace dom {

class Lock : public common::Singleton<Lock> {
  friend class common::Singleton<Lock>;

  public: class AutoLock : public base::AutoLock {
    public: AutoLock(const char* filename, int line_number)
        : base::AutoLock(*Lock::instance().lock()) {
      DVLOG(1) << "Lock dom at " << filename << "(" << std::dec <<
          line_number << ")";
      Lock::instance().locker_filename_ = filename;
      Lock::instance().locker_line_number_ = line_number;
    }

    public: ~AutoLock() {
      DVLOG(1) << "Unlock dom at " << Lock::instance().locker_filename_ <<
        "(" << std::dec << Lock::instance().locker_line_number_ << ")";
    }

    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };
  friend class AutoLock;

  private: std::unique_ptr<base::Lock> lock_;
  private: const char* locker_filename_;
  private: int locker_line_number_;

  private: Lock();
  public: virtual ~Lock() = default;

  public: base::Lock* lock() const { return lock_.get(); }

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

}  // namespace dom

#define ASSERT_DOM_IS_LOCKED() \
  dom::Lock::instance().lock()->AssertAcquired()

#define DOM_AUTO_LOCK_SCOPE() \
  dom::Lock::AutoLock dom_lock_scope(__FILE__, __LINE__)

#define DOM_AUTO_UNLOCK_SCOPE() \
  base::AutoUnlock dom_lock_scope(*dom::Lock::instance().lock())

#endif //!defined(INCLUDE_evita_dom_lock_h)
