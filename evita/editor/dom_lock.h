// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_EDITOR_DOM_LOCK_H_
#define EVITA_EDITOR_DOM_LOCK_H_

#include "base/location.h"
#include "base/logging.h"
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable : 4625 4626)
#include "base/threading/thread_checker.h"
#pragma warning(pop)

namespace editor {

class DomLock {
 public:
  typedef tracked_objects::Location Location;

  class AutoLock {
   public:
    explicit AutoLock(const Location& location);
    ~AutoLock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };

  class AutoTryLock {
   public:
    explicit AutoTryLock(const Location& location);
    ~AutoTryLock();

    bool locked() const { return locked_; }

   private:
    bool locked_;

    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };

  class AutoUnlock {
   public:
    explicit AutoUnlock(const Location& location);
    ~AutoUnlock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };

  DomLock();
  ~DomLock();

  static DomLock* instance();
  const Location& location() const;
  bool locked() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    return locked_;
  }

  void Acquire(const Location& location);
  void AssertLocked(const Location& location);
  void Release(const Location& location);
  bool TryLock(const Location& location);

 private:
  friend class AutoLock;
  friend class AutoTryLock;
  friend class AutoUnlock;

  bool locked_;
  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(DomLock);
};

}  // namespace editor

#define UI_ASSERT_DOM_LOCKED() \
  editor::DomLock::instance()->AssertLocked(FROM_HERE);

#define UI_DOM_AUTO_LOCK_SCOPE() editor::DomLock::AutoLock dom_scope(FROM_HERE)

#define UI_DOM_AUTO_TRY_LOCK_SCOPE(scope_name) \
  editor::DomLock::AutoTryLock scope_name(FROM_HERE)

#define UI_DOM_AUTO_UNLOCK_SCOPE() \
  editor::DomLock::AutoUnlock dom_scope(FROM_HERE)

#endif  // EVITA_EDITOR_DOM_LOCK_H_
