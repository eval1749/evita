// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_dom_lock_h)
#define INCLUDE_evita_editor_dom_lock_h

#include "base/location.h"
#include "base/logging.h"
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/threading/thread_checker.h"
#pragma warning(pop)

namespace editor {

class DomLock {
  private: typedef tracked_objects::Location Location;

  public: class AutoLock {
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

  public: class AutoUnlock {
    public: AutoUnlock(const Location& location);
    public: ~AutoUnlock();
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };
  friend class AutoUnlock;

  private: bool locked_;
  private: base::ThreadChecker thread_checker_;

  public: DomLock();
  public: ~DomLock();

  public: static DomLock* instance();
  public: const Location& location() const;
  public: bool locked() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    return locked_;
  }

  public: void Acquire(const Location& location);
  public: void AssertLocked(const Location& location);
  public: void Release(const Location& location);
  public: bool TryLock(const Location& location);

  DISALLOW_COPY_AND_ASSIGN(DomLock);
};

}  // namespace editor

#define UI_ASSERT_DOM_LOCKED() \
  editor::DomLock::instance()->AssertLocked(FROM_HERE);

#define UI_DOM_AUTO_LOCK_SCOPE() \
  editor::DomLock::AutoLock dom_scope(FROM_HERE)

#define UI_DOM_AUTO_TRY_LOCK_SCOPE(scope_name) \
  editor::DomLock::AutoTryLock scope_name(FROM_HERE)

#define UI_DOM_AUTO_UNLOCK_SCOPE() \
  editor::DomLock::AutoUnlock dom_scope(FROM_HERE)

#endif //!defined(INCLUDE_evita_editor_dom_lock_h)
