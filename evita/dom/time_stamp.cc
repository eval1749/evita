// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/time_stamp.h"

namespace dom {

TimeStamp::TimeStamp(double value) : value_(value) {
}

TimeStamp::TimeStamp(const TimeStamp& other) : value_(other.value_) {
}

TimeStamp::TimeStamp() : value_(0.0) {
}

TimeStamp::~TimeStamp() {
}

bool TimeStamp::operator==(const TimeStamp& other) const {
  return value_ == other.value_;
}

bool TimeStamp::operator!=(const TimeStamp& other) const {
  return value_ != other.value_;
}

bool TimeStamp::operator<(const TimeStamp& other) const {
  return value_ < other.value_;
}


bool TimeStamp::operator<=(const TimeStamp& other) const {
  return value_ <= other.value_;
}

bool TimeStamp::operator>(const TimeStamp& other) const {
  return value_ > other.value_;
}

bool TimeStamp::operator>=(const TimeStamp& other) const {
  return value_ >= other.value_;
}

}  // namespace dom
