// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/lock.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoLock
//
Lock::AutoLock::AutoLock(const char* filename, int line_number)
    : base::AutoLock(*Lock::instance().lock()) {
  DVLOG(1) << "Lock dom at " << filename << "(" << std::dec <<
      line_number << ")";
  Lock::instance().locker_filename_ = filename;
  Lock::instance().locker_line_number_ = line_number;
}

Lock::AutoLock::~AutoLock() {
  DVLOG(1) << "Unlock dom at " << Lock::instance().locker_filename_ <<
    "(" << std::dec << Lock::instance().locker_line_number_ << ")";
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoTryLock
//
Lock::AutoTryLock::AutoTryLock(const char* filename, int line_number)
    : locked_(Lock::instance().lock()->Try()) {
  DVLOG(1) << "Lock dom at " << filename << "(" << std::dec <<
      line_number << ")";
  Lock::instance().locker_filename_ = filename;
  Lock::instance().locker_line_number_ = line_number;
}

Lock::AutoTryLock::~AutoTryLock() {
  if (locked_)
    Lock::instance().lock()->Release();
  DVLOG(1) << "Unlock dom at " << Lock::instance().locker_filename_ <<
    "(" << std::dec << Lock::instance().locker_line_number_ << ")";
}

//////////////////////////////////////////////////////////////////////
//
// Lock::AutoUnlock
//
Lock::AutoUnlock::AutoUnlock(const char* filename, int line_number)
    : base::AutoUnlock(*Lock::instance().lock()) {
  DVLOG(1) << "Unlock dom at " << filename << "(" << std::dec <<
      line_number << ")";
  Lock::instance().locker_filename_ = filename;
  Lock::instance().locker_line_number_ = line_number;
}

Lock::AutoUnlock::~AutoUnlock() {
  DVLOG(1) << "Lock dom at " << Lock::instance().locker_filename_ <<
    "(" << std::dec << Lock::instance().locker_line_number_ << ")";
}

//////////////////////////////////////////////////////////////////////
//
// Lock
//
Lock::Lock()
    : lock_(new base::Lock()),
      locker_filename_(nullptr),
      locker_line_number_(0) {
}

}  // namespace dom
