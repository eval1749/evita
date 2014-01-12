// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_time_stamp_h)
#define INCLUDE_evita_dom_time_stamp_h

#include "evita/v8_glue/converter.h"

namespace dom {

class TimeStamp {
  public: typedef double ValueType;
  private: ValueType value_;

  public: explicit TimeStamp(double value);
  public: TimeStamp(const TimeStamp& other);
  public: TimeStamp();
  public: ~TimeStamp();

  public: operator ValueType() const { return value_; }

  public: TimeStamp& operator=(ValueType value) {
    value_ = value;
    return *this;
  }
  public: bool operator==(const TimeStamp& other) const;
  public: bool operator!=(const TimeStamp& other) const;
  public: bool operator<(const TimeStamp& other) const;
  public: bool operator<=(const TimeStamp& other) const;
  public: bool operator>(const TimeStamp& other) const;
  public: bool operator>=(const TimeStamp& other) const;

  public: static TimeStamp Now();
};

}  // namespace dom

namespace gin {
template<>
struct Converter<dom::TimeStamp> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const dom::TimeStamp& time_stamp);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     dom::TimeStamp* out);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_time_stamp_h)
