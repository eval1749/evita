// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_base_h)
#define INCLUDE_evita_text_range_base_h

namespace text {

class RangeSetBase;

class RangeBase {
  friend class RangeSetBase;

  private: Posn end_;
  private: Posn start_;

  protected: RangeBase(Posn start, Posn end);
  protected: RangeBase(const RangeBase& other);
  protected: ~RangeBase();

  public: Posn end() const { return end_; }
  public: Posn start() const { return start_; }

  public: bool Contains(Posn offset) const;
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_range_base_h)
