// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_lock_h)
#define INCLUDE_evita_dom_lock_h

#include <memory>

#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"

namespace editor {
class DomLock;
}

namespace dom {

class Lock : public common::Singleton<Lock> {
  friend class common::Singleton<Lock>;
  friend class editor::DomLock;

  public: class AutoLock : public base::AutoLock {
    public: AutoLock(const char* filename, int line_number);
    public: ~AutoLock();
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };
  friend class AutoLock;

  public: class AutoTryLock {
    private: bool locked_;
    public: AutoTryLock(const char* filename, int line_number);
    public: ~AutoTryLock();
    public: bool locked() const { return locked_; }
    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };
  friend class AutoTryLock;

  public: class AutoUnlock : public base::AutoUnlock {
    public: AutoUnlock(const char* filename, int line_number);
    public: ~AutoUnlock();
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };
  friend class AutoUnlock;

  private: std::unique_ptr<base::Lock> lock_;
  private: const char* locker_filename_;
  private: int locker_line_number_;

  private: Lock();
  public: virtual ~Lock() = default;

  private: base::Lock* lock() const { return lock_.get(); }

  public: void AssertAcquired() { lock_->AssertAcquired(); }

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

}  // namespace dom

#define ASSERT_DOM_LOCKED() \
  dom::Lock::instance()->AssertAcquired()

#define DOM_AUTO_LOCK_SCOPE() \
  dom::Lock::AutoLock dom_lock_scope(__FILE__, __LINE__)

#define DOM_AUTO_UNLOCK_SCOPE() \
  dom::Lock::AutoUnlock dom_lock_scope(__FILE__, __LINE__)

#define DOM_TRY_LOCK_SCOPE(name) \
  dom::Lock::AutoTryLock name(__FILE__, __LINE__)

#endif //!defined(INCLUDE_evita_dom_lock_h)
