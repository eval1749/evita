// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/time_stamp.h"

#include "base/time/time.h"

namespace dom {

TimeStamp::TimeStamp(double value) : value_(value) {}

TimeStamp::TimeStamp(const TimeStamp& other) : value_(other.value_) {}

TimeStamp::TimeStamp() : value_(0.0) {}

TimeStamp::~TimeStamp() {}

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

TimeStamp TimeStamp::Now() {
  return TimeStamp(base::Time::Now().ToDoubleT());
}

}  // namespace dom

namespace gin {

v8::Handle<v8::Value> Converter<dom::TimeStamp>::ToV8(
    v8::Isolate* isolate,
    const dom::TimeStamp& time_stamp) {
  return gin::ConvertToV8(isolate,
                          static_cast<dom::TimeStamp::ValueType>(time_stamp));
}

bool Converter<dom::TimeStamp>::FromV8(v8::Isolate* isolate,
                                       v8::Handle<v8::Value> val,
                                       dom::TimeStamp* out) {
  dom::TimeStamp::ValueType value;
  if (!gin::ConvertFromV8(isolate, val, &value))
    return false;
  *out = dom::TimeStamp(value);
  return true;
}

}  // namespace gin
