// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_time_stamp_h)
#define INCLUDE_evita_dom_time_stamp_h

namespace dom {

class TimeStamp {
  private: double value_;

  public: explicit TimeStamp(double value);
  public: TimeStamp(const TimeStamp& other);
  public: TimeStamp();
  public: ~TimeStamp();

  public: operator double() const { return value_; }

  public: bool operator==(const TimeStamp& other) const;
  public: bool operator!=(const TimeStamp& other) const;
  public: bool operator<(const TimeStamp& other) const;
  public: bool operator<=(const TimeStamp& other) const;
  public: bool operator>(const TimeStamp& other) const;
  public: bool operator>=(const TimeStamp& other) const;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_time_stamp_h)
