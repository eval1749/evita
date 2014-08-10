// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_lock_h)
#define INCLUDE_evita_dom_lock_h

#include <memory>

#include "base/location.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"

namespace dom {

class Lock : public common::Singleton<Lock> {
  DECLARE_SINGLETON_CLASS(Lock);

  private: typedef tracked_objects::Location Location;

  public: class AutoLock : public base::AutoLock {
    public: AutoLock(const Location& location);
    public: ~AutoLock();
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };
  friend class AutoLock;

  public: class AutoTryLock {
    private: bool locked_;
    public: AutoTryLock(const Location& location);
    public: ~AutoTryLock();
    public: bool locked() const { return locked_; }
    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };
  friend class AutoTryLock;

  public: class AutoUnlock : public base::AutoUnlock {
    public: AutoUnlock(const Location& location);
    public: ~AutoUnlock();
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };
  friend class AutoUnlock;

  private: Location location_;
  private: std::unique_ptr<base::Lock> lock_;
  private: bool locked_by_dom_;

  private: Lock();
  public: virtual ~Lock();

  public: bool locked_by_dom() const { return locked_by_dom_; }
  public: void set_locked_by_dom() { locked_by_dom_ = true; }
  public: const Location& location() const { return location_; }
  private: base::Lock* lock() const { return lock_.get(); }

  public: void Acquire(const Location& location);
  public: void AssertAcquired() { lock_->AssertAcquired(); }
  public: void Release(const Location& location);
  public: bool TryLock(const Location& location);

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

}  // namespace dom

#if _DEBUG
#define ASSERT_DOM_LOCKED() \
  dom::Lock::instance()->AssertAcquired()
#else
#define ASSERT_DOM_LOCKED()
#endif

#define DOM_AUTO_LOCK_SCOPE() \
  dom::Lock::AutoLock dom_lock_scope(FROM_HERE)

#define DOM_AUTO_UNLOCK_SCOPE() \
  dom::Lock::AutoUnlock dom_lock_scope(FROM_HERE)

#define DOM_TRY_LOCK_SCOPE(name) \
  dom::Lock::AutoTryLock name(FROM_HERE)

#endif //!defined(INCLUDE_evita_dom_lock_h)
