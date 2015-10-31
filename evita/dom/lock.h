// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_LOCK_H_
#define EVITA_DOM_LOCK_H_

#include <memory>

#include "base/location.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"

namespace dom {

class Lock final : public common::Singleton<Lock> {
  DECLARE_SINGLETON_CLASS(Lock);

 public:
  using Location = tracked_objects::Location;

  class AutoLock final {
   public:
    explicit AutoLock(const Location& location);
    ~AutoLock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };

  class AutoTryLock final {
   public:
    explicit AutoTryLock(const Location& location);
    ~AutoTryLock();

    bool locked() const { return locked_; }

   private:
    bool locked_;
    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };

  class AutoUnlock final : public base::AutoUnlock {
   public:
    explicit AutoUnlock(const Location& location);
    ~AutoUnlock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };

  virtual ~Lock();

  bool locked_by_dom() const { return locked_by_dom_; }
  void set_locked_by_dom() { locked_by_dom_ = true; }
  const Location& location() const { return location_; }
  base::Lock* lock() const { return lock_.get(); }

  void Acquire(const Location& location);
  void AssertAcquired() { lock_->AssertAcquired(); }
  void Release(const Location& location);
  bool TryLock(const Location& location);

 private:
  friend class AutoLock;
  friend class AutoTryLock;
  friend class AutoUnlock;

  Lock();

  Location location_;
  std::unique_ptr<base::Lock> lock_;
  bool locked_by_dom_;

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

}  // namespace dom

#if _DEBUG
#define ASSERT_DOM_LOCKED() dom::Lock::instance()->AssertAcquired()
#else
#define ASSERT_DOM_LOCKED()
#endif

#define DOM_AUTO_LOCK_SCOPE() dom::Lock::AutoLock dom_lock_scope(FROM_HERE)

#define DOM_AUTO_UNLOCK_SCOPE() dom::Lock::AutoUnlock dom_lock_scope(FROM_HERE)

#define DOM_TRY_LOCK_SCOPE(name) dom::Lock::AutoTryLock name(FROM_HERE)

#endif  // EVITA_DOM_LOCK_H_
