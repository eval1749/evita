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

namespace gin {

v8::Handle<v8::Value> Converter<dom::TimeStamp>::ToV8(
    v8::Isolate* isolate, const dom::TimeStamp& time_stamp) {
  return gin::ConvertToV8(isolate, time_stamp);
}

bool Converter<dom::TimeStamp>::FromV8(v8::Isolate* isolate,
                                       v8::Handle<v8::Value> val,
                                       dom::TimeStamp* out) {
  double double_value;
  if (!gin::ConvertFromV8(isolate, val, &double_value))
    return false;
  *out = dom::TimeStamp(double_value);
  return true;
}

}  // namespace gin
