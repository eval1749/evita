// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_TIME_STAMP_H_
#define EVITA_DOM_TIME_STAMP_H_

#include "evita/v8_glue/converter.h"

namespace dom {

class TimeStamp final {
 public:
  typedef double ValueType;

  explicit TimeStamp(double value);
  TimeStamp(const TimeStamp& other);
  TimeStamp();
  ~TimeStamp();

  operator ValueType() const { return value_; }

  TimeStamp& operator=(ValueType value) {
    value_ = value;
    return *this;
  }
  bool operator==(const TimeStamp& other) const;
  bool operator!=(const TimeStamp& other) const;
  bool operator<(const TimeStamp& other) const;
  bool operator<=(const TimeStamp& other) const;
  bool operator>(const TimeStamp& other) const;
  bool operator>=(const TimeStamp& other) const;

  static TimeStamp Now();

 private:
  ValueType value_;
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::TimeStamp> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const dom::TimeStamp& time_stamp);
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val,
                     dom::TimeStamp* out);
};
}  // namespace gin

#endif  // EVITA_DOM_TIME_STAMP_H_
