// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_DOM_LOCK_H_
#define EVITA_EDITOR_DOM_LOCK_H_

#include "base/location.h"
#include "base/logging.h"
#include "base/threading/thread_checker.h"

namespace editor {

class DomLock {
 public:
  class AutoLock {
   public:
    explicit AutoLock(const base::Location& location);
    ~AutoLock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };

  class AutoTryLock {
   public:
    explicit AutoTryLock(const base::Location& location);
    ~AutoTryLock();

    bool locked() const { return locked_; }

   private:
    bool locked_;

    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };

  class AutoUnlock {
   public:
    explicit AutoUnlock(const base::Location& location);
    ~AutoUnlock();

   private:
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };

  DomLock();
  ~DomLock();

  const base::Location& location() const;
  bool locked() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    return locked_;
  }

  void Acquire(const base::Location& location);
  void AssertLocked(const base::Location& location);
  void Release(const base::Location& location);
  bool TryLock(const base::Location& location);

  static DomLock* GetInstance();

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
  editor::DomLock::GetInstance()->AssertLocked(FROM_HERE);

#define UI_DOM_AUTO_LOCK_SCOPE() editor::DomLock::AutoLock dom_scope(FROM_HERE)

#define UI_DOM_AUTO_TRY_LOCK_SCOPE(scope_name) \
  editor::DomLock::AutoTryLock scope_name(FROM_HERE)

#define UI_DOM_AUTO_UNLOCK_SCOPE() \
  editor::DomLock::AutoUnlock dom_scope(FROM_HERE)

#endif  // EVITA_EDITOR_DOM_LOCK_H_
