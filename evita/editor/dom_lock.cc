// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/editor/dom_lock.h"

#include "evita/dom/lock.h"
#include "evita/editor/application.h"

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoLock
//
DomLock::AutoLock::AutoLock(const char* filename, int line_number,
                            const char* function_name) {
  DVLOG(1) << "Lock dom at " << filename << "(" <<
      line_number << ") " << function_name;
  auto const lock = DomLock::instance();
  lock->Lock();
  lock->locker_filename_ = filename;
  lock->locker_function_name_ = function_name;
  lock->locker_line_number_ = line_number;
}

DomLock::AutoLock::~AutoLock() {
  auto const lock = DomLock::instance();
  DVLOG(1) << "Unlock dom at " << lock->locker_filename_ <<
      "(" << lock->locker_line_number_ << ") " << lock->locker_function_name_;
  lock->Unlock();
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoTryLock
//
DomLock::AutoTryLock::AutoTryLock(const char* filename, int line_number,
                                  const char* function_name)
    : locked_(DomLock::instance()->TryLock()) {
  auto const lock = DomLock::instance();
  DVLOG(1) << "TryLock dom by " << lock->locker_function_name_ <<
      " at " << lock->locker_filename_ <<
      "(" << lock->locker_line_number_ << ") ";
  lock->locker_filename_ = filename;
  lock->locker_function_name_ = function_name;
  lock->locker_line_number_ = line_number;
}

DomLock::AutoTryLock::~AutoTryLock() {
  if (!locked_)
    return;
  auto const lock = DomLock::instance();
  lock->Unlock();
  DVLOG(1) << "Unlock dom by " << lock->locker_function_name_ <<
      " at " << lock->locker_filename_ <<
      "(" << lock->locker_line_number_ << ") ";
}

//////////////////////////////////////////////////////////////////////
//
// DomLock::AutoUnlock
//
DomLock::AutoUnlock::AutoUnlock(const char* filename, int line_number,
                                const char* function_name) {
  auto const lock = DomLock::instance();
  DVLOG(1) << "Unlock dom by " << lock->locker_function_name_ <<
      " at " << lock->locker_filename_ <<
      "(" << lock->locker_line_number_ << ") ";
  lock->Unlock();
  lock->locker_filename_ = filename;
  lock->locker_function_name_ = function_name;
  lock->locker_line_number_ = line_number;
}

DomLock::AutoUnlock::~AutoUnlock() {
  auto const lock = DomLock::instance();
  lock->Lock();
  DVLOG(1) << "Lock dom by " << lock->locker_function_name_ <<
      " at " << lock->locker_filename_ <<
      "(" << lock->locker_line_number_ << ") ";
}

//////////////////////////////////////////////////////////////////////
//
// DomLock
//
DomLock::DomLock() : locked_(false), locker_filename_(""),
    locker_function_name_(""), locker_line_number_(0) {
}

DomLock* DomLock::instance() {
  return Application::instance()->dom_lock();
}

void DomLock::AssertLocked() {
  if (locked_)
    return;
  LOG(ERROR) << "Locked by " << locker_function_name_ <<
    locker_filename_ << "(" << locker_line_number_ << ") ";
  NOTREACHED();
}

void DomLock::Lock() {
  DCHECK(thread_checker_.CalledOnValidThread());
  dom::Lock::instance()->lock()->Acquire();
  locked_ = true;
}

bool DomLock::TryLock() {
  DCHECK(thread_checker_.CalledOnValidThread());
  locked_ = dom::Lock::instance()->lock()->Try();
  return locked_;
}

void DomLock::Unlock() {
  DCHECK(thread_checker_.CalledOnValidThread());
  dom::Lock::instance()->lock()->Release();
  locked_ = false;
}

}  // namespace editor
